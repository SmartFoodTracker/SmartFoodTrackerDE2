/** @file   input_tasks.c
 *  @brief  Routines for food inventory tracker data entry
 *
 *  Wait for either audio-clip or barcode to be entered;
 *  Send data to server for processing (translate to plain text);
 *  Display item plain text and have user click a button to add or remove the
 *  item.
 *
 *  @author Andrew Bradshaw (abradsha), Kyle O'Shaughnessy (koshaugh)
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

// System routines
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include "system.h"
#include "includes.h"
#include "altera_avalon_pio_regs.h"

// Device driver routines
#include "buttons.h"
#include "barcode_scanner.h"
#include "microphone.h"
#include "altera_up_avalon_character_lcd.h"
#include "client.h"

// Parsing
#include "word_parser.h"

// Web server routines
#include "alt_error_handler.h"
#include "web_server.h"
#include "dm9000a.h"

// Nichestack
#include "ipport.h"
#include "libport.h"
#include "osport.h"
#include "tcpport.h"
#include "net.h"

#include "input_tasks.h"

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

#define TASK_STACKSIZE              2048
#define MUTEX_PRIORITY              6
#define BARCODE_TASK_PRIORITY       7
#define MICROPHONE_TASK_PRIORITY    8
#define ITEM_NAME_MAX_LENGTH        256

/*****************************************************************************/
/* Globals                                                                   */
/*****************************************************************************/

OS_EVENT   *pConfirmationMutex;
OS_STK      pBarcodeTaskStack[TASK_STACKSIZE];
OS_STK      pMicrophoneTaskStack[TASK_STACKSIZE];

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

/**
 * @brief      Microphone task; waits on voice, non blocking pends on
 *             shared transmit mutex, then goes through confirmation process
 *
 * @param[in]  pData  Pointer to task context, confirmation buttons in this case
 */
void
MicrophoneTask(void* pData)
{
    INT8U               status      = OS_NO_ERR;
    Microphone         *pMicrophone = NULL;
    Buttons            *pButtons    = (Buttons *) pData;
    Linear16Recording   exportedRecording;
    char                audio_string[ITEM_NAME_MAX_LENGTH];

    // Setup push-to-talk microphone
    pMicrophone = microphoneCreate(AUDIO_CORE_NAME,
                                   AUDIO_CORE_IRQ,
                                   SWITCH_BASE,
                                   SWITCH_IRQ);
    if (pMicrophone == NULL)
    {
        status = OS_ERR_PDATA_NULL;
        printf("Microphone setup failed.\n");
    }

    while (pMicrophone != NULL)
    {
        // Record audio clip (wait on push-to-talk)
        microphoneWaitAndBeginRecording(pMicrophone);
        microphoneWaitAndFinishRecording(pMicrophone);
        microphoneExportLinear16(pMicrophone, &exportedRecording);

        // Try to enter item confirmation workflow
        OSMutexPend(pConfirmationMutex, 1, &status);
        if (status == OS_ERR_NONE)
        {
            translate_audio(exportedRecording.pRecording, exportedRecording.size * 2, audio_string);
            printf("Voice decoded: %s\n", audio_string);
            ConfirmItem(audio_string, pButtons);
            OSMutexPost(pConfirmationMutex);
        }
        else
        {
            printf("discarding data\n");
        }
    }
} // MicrophoneTask

/*****************************************************************************/

/**
 * @brief      Barcode task; waits on barcode scan, non blocking pends on
 *             shared transmit mutex, then goes through confirmation process
 *
 * @param[in]  pData  Pointer to task context, confirmation buttons in this case
 */
void
BarcodeTask(void* pData)
{
    INT8U           status          = OS_NO_ERR;
    BarcodeScanner *pBarcodeScanner = NULL;
    Buttons        *pButtons        = (Buttons *) pData;
    Barcode         barcode;
    char            pItemString[ITEM_NAME_MAX_LENGTH];

    // Create and initialize barcode scanner
    pBarcodeScanner = barcodeScannerCreate(BARCODE_SCANNER_PS2_NAME,
                                           BARCODE_SCANNER_PS2_BASE,
                                           BARCODE_SCANNER_PS2_IRQ);
    if (pBarcodeScanner == NULL)
    {
        printf("Barcode scanner setup failed.\n");
    }

    while (pBarcodeScanner != NULL)
    {
        // Wait for barcode scanner to produce a new barcode
        barcodeScannerDecode(pBarcodeScanner, &barcode);

        // Try to enter item confirmation workflow
        OSMutexPend(pConfirmationMutex, 1, &status);
        if (status == OS_ERR_NONE) {
            printf("Barcode: %s\n", barcode.pString);
            translate_barcode(barcode.pString, pItemString);
            printf("Barcode decoded: %s\n", pItemString);
            ConfirmItem(pItemString, pButtons);
            OSMutexPost(pConfirmationMutex);
        }
        else
        {
            printf("discarding data\n");
        }
    }
} // BarcodeTask

/*****************************************************************************/

/**
 * @brief      Display item on LCD and process button response
 *
 * @param[in]  pItemName  String representing item to be added
 */
void
ConfirmItem(char* pItemName, Buttons *pButtons)
{
    alt_up_character_lcd_dev   *pLCD    = NULL;
    Button                      button  = ButtonMax;
    Command                     command = CommandNothing;
    char pItemNameNoCommand[ITEM_NAME_MAX_LENGTH];
    char pItemNameNoQuantity[ITEM_NAME_MAX_LENGTH];
    int amount = 1;
    

    if ((pLCD = alt_up_character_lcd_open_dev(CHARACTER_LCD_NAME)) != NULL)
    {
        // Clear LCD
        alt_up_character_lcd_init(pLCD);
        alt_up_character_lcd_set_cursor_pos(pLCD, 0, 0);

        // Write item string to LCD
        alt_up_character_lcd_string(pLCD, pItemName);

        command = parse_command(pItemName, pItemNameNoCommand);
        amount = parse_number(pItemNameNoCommand, pItemNameNoQuantity);


        if (command == CommandNothing) {
            // Get confirmation response
            buttonsEnableAll(pButtons);
            button = buttonsGetButtonPress(pButtons);
            buttonsDisableAll(pButtons);
        }

        // Add or remove item depending on response
        if (button == ButtonAdd || command == CommandAdd)
        {
            add_item(pItemNameNoQuantity, amount);
            displayStatusEx(FITStatusItemAdded, pItemNameNoCommand);
            OSTimeDlyHMSM(0,0,2,0);
        }
        else if (button == ButtonRemove || command == CommandRemove)
        {
            add_item(pItemNameNoQuantity, -1 * amount);
            displayStatusEx(FITStatusItemRemoved, pItemNameNoCommand);
            OSTimeDlyHMSM(0,0,2,0);
        }
        displayStatus(FITStatusReady);
    }
    else
    {
        printf("LCD setup failed\n");
    }
} // ConfirmItem

/*****************************************************************************/
/**
 * @brief      Set LCD and status light messages given a particular status
 *
 * @param[in]  status  FIT status indicating state of the client board
 */
void
displayStatus(FITStatus status)
{
    displayStatusEx(status, NULL);
} // dispalyStatus

/*****************************************************************************/

/**
 * @brief      Extension to displayStatus that allows optional messages to be
 *             appended
 *
 * @param[in]  status  FIT status indicating state of the client board
 * @param[in]  pOptionalString  Additional information that may be appended
 *                              to status message depending on status code
 */
void
displayStatusEx(FITStatus status, char *pOptionalString)
{
    alt_up_character_lcd_dev *pLCD = NULL;

    if ((pLCD = alt_up_character_lcd_open_dev(CHARACTER_LCD_NAME)) != NULL)
    {
        // Clear LCD
        alt_up_character_lcd_init(pLCD);
        alt_up_character_lcd_set_cursor_pos(pLCD, 0, 0);

        switch (status)
        {
        case FITStatusReady:

            // Write Messages
            alt_up_character_lcd_string(pLCD, FIT_MSG_READY);
            printf("%s\n", FIT_MSG_READY);

            // Set LEDs
            IOWR(RED_LEDS_BASE,   0, 0x0);
            IOWR(GREEN_LEDS_BASE, 0, 0x1);

            break;

        case FITStatusSetupFailed:

            // Write Messages
            alt_up_character_lcd_string(pLCD, FIT_MSG_SETUP_FAILED);
            printf("%s\n", FIT_MSG_SETUP_FAILED);

            // Set LEDs
            IOWR(RED_LEDS_BASE,   0, 0x1);
            IOWR(GREEN_LEDS_BASE, 0, 0x0);

            break;

        case FITStatusItemAdded:

            // Write Messages
            alt_up_character_lcd_string(pLCD, FIT_MSG_ITEM_ADDED);
            if (pOptionalString)
            {
                printf("%s: %s\n", FIT_MSG_ITEM_ADDED, pOptionalString);
            }
            else
            {
                printf("%s\n", FIT_MSG_ITEM_ADDED);
            }

            break;
        case FITStatusItemRemoved:

            // Write Messages
            alt_up_character_lcd_string(pLCD, FIT_MSG_ITEM_REMOVED);
            if (pOptionalString)
            {
                printf("%s: %s\n", FIT_MSG_ITEM_REMOVED, pOptionalString);
            }
            else
            {
                printf("%s\n", FIT_MSG_ITEM_REMOVED);
            }

            break;

        default:
            break;
        }
    }
    else
    {
        printf("LCD setup failed\n");
    }
} // DisplayStatusEx

/*****************************************************************************/

/**
 * @brief      Routine which sets up input tasks, sync objects, and shared
 *             devices
 */
void
FITSetup()
{
    INT8U       status      = OS_NO_ERR;
    Buttons    *pButtons    = NULL;

    // Initialize input synchronization mutex
    pConfirmationMutex = OSMutexCreate(MUTEX_PRIORITY, &status);
    if (status != OS_NO_ERR)
    {
        printf("Mutex creation failed.\n");
    }

    // Create Buttons object
    if (status == OS_NO_ERR)
    {
        pButtons = buttonsCreate();
        if (pButtons != NULL)
        {
            // Initialize all buttons
            if (status == OS_NO_ERR)
                status = buttonsInitButton(pButtons,
                                           ButtonAdd,
                                           ADD_BUTTON_BASE,
                                           ADD_BUTTON_IRQ);
            if (status == OS_NO_ERR)
                status = buttonsInitButton(pButtons,
                                           ButtonCancel,
                                           CANCEL_BUTTON_BASE,
                                           CANCEL_BUTTON_IRQ);
            if (status == OS_NO_ERR)
                status = buttonsInitButton(pButtons,
                                           ButtonRemove,
                                           REMOVE_BUTTON_BASE,
                                           REMOVE_BUTTON_IRQ);
        }
        else
        {
            status = OS_ERR_PDATA_NULL;
            printf("Buttons creation failed.\n");
        }
    }

    // Initialize input processing tasks
    if (status == OS_NO_ERR)
    {
        if (status == OS_NO_ERR)
        {
            status = OSTaskCreateExt(MicrophoneTask,
                                     pButtons,
                                     &pMicrophoneTaskStack[TASK_STACKSIZE-1],
                                     MICROPHONE_TASK_PRIORITY,
                                     MICROPHONE_TASK_PRIORITY,
                                     pMicrophoneTaskStack,
                                     TASK_STACKSIZE,
                                     NULL,
                                     0);
            if (status != OS_NO_ERR)
            {
                printf("LCDTask setup failed.\n");
            }
        }
        if (status == OS_NO_ERR)
        {
            status = OSTaskCreateExt(BarcodeTask,
                                     pButtons,
                                     &pBarcodeTaskStack[TASK_STACKSIZE-1],
                                     BARCODE_TASK_PRIORITY,
                                     BARCODE_TASK_PRIORITY,
                                     pBarcodeTaskStack,
                                     TASK_STACKSIZE,
                                     NULL,
                                     0);
            if (status != OS_NO_ERR)
            {
                printf("LCDTask setup failed.\n");
            }
        }
    }

    if (status == OS_NO_ERR)
    {
        displayStatus(FITStatusReady);
    }
    else
    {
        displayStatus(FITStatusSetupFailed);
    }
} // FITSetup

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

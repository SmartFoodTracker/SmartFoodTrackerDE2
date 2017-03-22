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

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

#define TASK_STACKSIZE              2048
#define MUTEX_PRIORITY              6
#define BARCODE_TASK_PRIORITY       7
#define MICROPHONE_TASK_PRIORITY    8
#define ITEM_NAME_MAX_LENGTH        64

/*****************************************************************************/
/* Globals                                                                   */
/*****************************************************************************/

Buttons *pButtons;
/* Declaration of the mutex to protect global buttons and lcd*/
OS_EVENT *confirmationMutex;

/* Declarations for creating a task with TK_NEWTASK.
 * All tasks which use NicheStack (those that use sockets) must be created this way.
 * TK_OBJECT macro creates the static task object used by NicheStack during operation.
 * TK_ENTRY macro corresponds to the entry point, or defined function name, of the task.
 * inet_taskinfo is the structure used by TK_NEWTASK to create the task.
 */

extern void MicrophoneTask();
extern void BarcodeTask();
extern void ConfirmItem();

TK_OBJECT(to_MicrophoneTask);
TK_ENTRY(MicrophoneTask);

struct inet_taskinfo mictask = {
            &to_MicrophoneTask,
            "microphone_task",
            MicrophoneTask,
            MICROPHONE_TASK_PRIORITY,
            TASK_STACKSIZE,
};

TK_OBJECT(to_BarcodeTask);
TK_ENTRY(BarcodeTask);

struct inet_taskinfo bartask = {
            &to_BarcodeTask,
            "barcode_task",
            BarcodeTask,
            BARCODE_TASK_PRIORITY,
            TASK_STACKSIZE,
};

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

/**
 * @brief      Microphone task; waits on voice, non blocking pends on
 *             shared transmit mutex, then goes through confirmation process
 *
 * @param      pData  Pointer to task context (NULL)
 */
void
MicrophoneTask(void* pData)
{
    INT8U status = OS_NO_ERR;
    Microphone *pMicrophone = NULL;
    Linear16Recording exportedRecording;
    char audio_string[ITEM_NAME_MAX_LENGTH];

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
        microphoneWaitAndBeginRecording(pMicrophone);
        microphoneWaitAndFinishRecording(pMicrophone);
        microphoneExportLinear16(pMicrophone, &exportedRecording);

        // Playback recording to line-out as a debugging safety-net
        microphonePlaybackRecording(pMicrophone);

        // Nonblocking mutex to throw away data while blocked
        OSMutexPend(confirmationMutex, 1, &status);
        if (status == OS_ERR_NONE) {
            translate_audio(exportedRecording.pRecording, exportedRecording.size * 2, audio_string);
            printf("Voice decoded: %s\n", audio_string);
            ConfirmItem(audio_string);
            OSMutexPost(confirmationMutex);
        } else {
            printf("discarding data\n");
        }
    }

    if (pMicrophone)
    {
        microphoneDestroy(pMicrophone);
    }
} // MicrophoneTask

/*****************************************************************************/

/**
 * @brief      Barcode task; waits on barcode scan, non blocking pends on
 *             shared transmit mutex, then goes through confirmation process
 *
 * @param[in]  pData  Pointer to task context (NULL)
 */
void
BarcodeTask(void* pData)
{
    INT8U status = OS_NO_ERR;
    BarcodeScanner *pBarcodeScanner = NULL;
    Barcode barcode;
    char pItemString[ITEM_NAME_MAX_LENGTH];

    // Create and initialize barcode scanner
    pBarcodeScanner = barcodeScannerCreate(BARCODE_SCANNER_PS2_NAME,
                                           BARCODE_SCANNER_PS2_BASE,
                                           BARCODE_SCANNER_PS2_IRQ);
    if (pBarcodeScanner == NULL)
    {
        printf("Barcode scanner setup failed.\n");
    }

    while (1)
    {
        barcodeScannerDecode(pBarcodeScanner, &barcode);
        // Nonblocking mutex to throw away data while blocked
        OSMutexPend(confirmationMutex, 1, &status);
        if (status == OS_ERR_NONE) {
            printf("Barcode: %s\n", barcode.pString);
            translate_barcode(barcode.pString, pItemString);
            printf("Barcode decoded: %s\n", pItemString);
            ConfirmItem(pItemString);
            OSMutexPost(confirmationMutex);
        } else {
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
ConfirmItem(char* pItemName)
{
    alt_up_character_lcd_dev   *pLCD    = NULL;
    Button                      button  = ButtonMax;

    if ((pLCD = alt_up_character_lcd_open_dev(CHARACTER_LCD_NAME)) != NULL)
    {
        // Clear LCD
        alt_up_character_lcd_init(pLCD);
        alt_up_character_lcd_set_cursor_pos(pLCD, 0, 0);

        // Write item string to LCD
        alt_up_character_lcd_string(pLCD, pItemName);

        // Get confirmation response
        buttonsEnableAll(pButtons);
        button = buttonsGetButtonPress(pButtons);
        buttonsDisableAll(pButtons);

        // Clear LCD
        alt_up_character_lcd_init(pLCD);
        alt_up_character_lcd_set_cursor_pos(pLCD, 0, 0);

        // Add or remove item depending on response
        if (button == ButtonAdd) {
            printf("added \"%s\"\n", pItemName);
            add_item(pItemName);
        } else if (button == ButtonRemove) {
            printf("removed \"%s\"\n", pItemName);
            remove_item(pItemName);
        }
    }
    else
    {
        printf("LCD setup failed\n");
    }
} // ConfirmItem

/*****************************************************************************/

/**
 * @brief      Routine which sets up input tasks and
 */
void
FITSetup()
{
    INT8U status = OS_NO_ERR;

    // Initialize input synchronization mutex
    confirmationMutex = OSMutexCreate(MUTEX_PRIORITY, &status);
    if (status != OS_NO_ERR)
    {
        printf("confirmationMutex creation failed.\n");
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

    if (status == OS_NO_ERR)
    {
        // Initialize input processing tasks
        if (TK_NEWTASK(&mictask) != 0)
        {
            status = OS_ERR_TASK_NOT_EXIST;
        }
        if (TK_NEWTASK(&bartask) != 0)
        {
            status = OS_ERR_TASK_NOT_EXIST;
        }
    }
} // FITSetup

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

/** @file   input_tasks.c
 *  @brief  Code for entering food into the food inventory tracking system
 *
 *  Wait for either audio or barcode to be entered
 *  Send them to server and have it translate it into text
 *  Display text and have user click a button to either add or remove the food
 *
 *
 *  @author Andrew Bradshaw (abradsha)
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "includes.h"

#include "buttons.h"
#include "barcode_scanner.h"
#include "client.h"
#include "microphone.h"
#include "altera_up_avalon_character_lcd.h"

/* Web Server definitions */
#include "alt_error_handler.h"
#include "web_server.h"
#include "dm9000a.h"

/* Nichestack definitions */
#include "ipport.h"
#include "libport.h"
#include "osport.h"
#include "tcpport.h"
#include "net.h"

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

#define TASK_STACKSIZE      2048
#define MUTEX_PRIORITY           6
#define MICROPHONE_TASK_PRIORITY   8
#define BARCODE_TASK_PRIORITY   7
#define ITEM_SIZE 64

/*****************************************************************************/
/* Globals                                                                   */
/*****************************************************************************/

Buttons *pButtons;
alt_up_character_lcd_dev *pLCD;
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
extern void DisplayText();

//extern struct net netstatic[STATIC_NETS];

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
 */
void MicrophoneTask(void* pData) {
    INT8U status = OS_NO_ERR;
    Microphone *pMicrophone = NULL;
    Linear16Recording exportedRecording;
    char audio_string[ITEM_SIZE];
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
    while (1)
    {
    	microphoneEnablePushToTalk(pMicrophone);
        microphoneWaitAndBeginRecording(pMicrophone);
        microphoneWaitAndFinishRecording(pMicrophone);
        microphoneDisablePushToTalk(pMicrophone);
        microphoneExportLinear16(pMicrophone, &exportedRecording);
        // Nonblocking mutex to throw away data while blocked
        OSMutexPend(confirmationMutex, 1, &status);
        if (status == OS_ERR_NONE) {
            translate_audio(exportedRecording.pRecording, exportedRecording.size * 2, audio_string);
            DisplayText(audio_string);
            OSMutexPost(confirmationMutex);
        } else {
        	printf("discarding data\n");
        }
    }
} // MicrophoneTask

/**
 * @brief      Barcode task; waits on barcode scan, non blocking pends on
 *             shared transmit mutex, then goes through confirmation process
 *
 */
void BarcodeTask(void* pData) {
    INT8U status = OS_NO_ERR;
    BarcodeScanner *pBarcodeScanner = NULL;
    Barcode barcode;
    char barcode_string[ITEM_SIZE];
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
            translate_barcode(barcode.pString, barcode_string);
            DisplayText(barcode_string);
            OSMutexPost(confirmationMutex);
        } else {
        	printf("discarding data\n");
        }
    }
} // BarcodeTask

/**
 * @brief      Display String on LCD and get button response based on user input
 *
 */
void DisplayText(char* item) {
    Button button = ButtonMax;
    // Clear LCD
    alt_up_character_lcd_init(pLCD);
    alt_up_character_lcd_set_cursor_pos(pLCD, 0, 0);

    // Write button string to LCD
    alt_up_character_lcd_string(pLCD, item);
    buttonsEnableAll(pButtons);
    button = buttonsGetButtonPress(pButtons);
    buttonsDisableAll(pButtons);

    // Clear LCD
    alt_up_character_lcd_init(pLCD);
    alt_up_character_lcd_set_cursor_pos(pLCD, 0, 0);

    if (button == ButtonAdd) {
        printf("added item\n");
        add_item(item);
    } else if (button == ButtonRemove) {
        printf("removed item\n");
        remove_item(item);
    }
}

/*****************************************************************************/

/**
 * @brief      Task to set up subtasks
 *
 */
void FITSetup()
{
    INT8U       status      = OS_NO_ERR;

    // Create Buttons object
    pButtons = buttonsCreate();
    if (pButtons == NULL)
    {
        printf("Buttons creation failed.\n");
    }

    // Initialize mutex
    confirmationMutex = OSMutexCreate(MUTEX_PRIORITY, &status);

    // Initialize all buttons
    buttonsInitButton(pButtons,
                        ButtonAdd,
                        ADD_BUTTON_BASE,
                                ADD_BUTTON_IRQ);

    buttonsInitButton(pButtons,
                        ButtonCancel,
                        CANCEL_BUTTON_BASE,
                                CANCEL_BUTTON_IRQ);

    buttonsInitButton(pButtons,
                        ButtonRemove,
                        REMOVE_BUTTON_BASE,
                                REMOVE_BUTTON_IRQ);
    // LCD setup
    if ((pLCD = alt_up_character_lcd_open_dev(CHARACTER_LCD_NAME)) == NULL)
    {
        printf("LCD setup failed\n");
    } else {
        // Clear LCD
        alt_up_character_lcd_init(pLCD);
        alt_up_character_lcd_set_cursor_pos(pLCD, 0, 0);
        alt_up_character_lcd_string(pLCD, "setup");
    }

    TK_NEWTASK(&mictask);
    TK_NEWTASK(&bartask);
} // FITSetup

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

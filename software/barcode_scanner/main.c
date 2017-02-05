#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "includes.h"
#include "io.h"
#include "altera_up_avalon_character_lcd.h"
#include "altera_up_avalon_ps2.h"
#include "altera_up_ps2_keyboard.h"

/* Definition of Task Stacks */
#define         TASK_STACKSIZE          2048

/* Definition of Task Priorities */
#define         LCD_TASK_PRIORITY       1

/* Definition of Queue properties */
#define         LCD_MESSAGE_QUEUE_SIZE  24

/* LCD Properties */
#define         LCD_WIDTH               16

/* Globals */
OS_STK          LCDTaskStack[TASK_STACKSIZE];

OS_EVENT       *pBarcodeKeyPressQueue;
void           *pBarcodeKeyPressQueueData[LCD_MESSAGE_QUEUE_SIZE];

/* Structures */
typedef struct _ENCODED_KEY_PRESS
{
    KB_CODE_TYPE    decodeMode;
    INT8U           encodedValue;
} ENCODED_KEY_PRESS;

typedef enum _DECODE_STATUS
{
    DECODE_STATUS_COMPLETE,
    DECODE_STATUS_NOT_COMPLETE
} DECODE_STATUS;

typedef enum _KEY_POSITION
{
    KEY_POSITION_UP,
    KEY_POSITION_DOWN
} KEY_POSITION;

DECODE_STATUS
decodeBarcode(char                 *pBarcodeBuffer,
              ENCODED_KEY_PRESS    *pEncodedKeyPress,
              KEY_POSITION         *pKeyPosition)
{
    char            keyPressString[16];
    DECODE_STATUS   status = DECODE_STATUS_NOT_COMPLETE;

    // Decode the keypress
    translate_make_code(pEncodedKeyPress->decodeMode, pEncodedKeyPress->encodedValue, keyPressString);

    if (strcmp(keyPressString, "L SHFT") != 0)
    {
        // update the key position from the last decode sequence
        if (*pKeyPosition == KEY_POSITION_UP)
        {
            *pKeyPosition = KEY_POSITION_DOWN;
            if (strcmp(keyPressString, "SPACE") == 0)
            {
                status = DECODE_STATUS_COMPLETE;
            }
            else
            {
                strncat(pBarcodeBuffer, keyPressString, strlen(keyPressString));
            }
        }
        else
        {
            *pKeyPosition = KEY_POSITION_UP;
        }
    }

    return status;
}

/* LCD update task, pends on a queue and writes frequency to LCD display */
void
LCDTask(void* pData)
{
    alt_up_character_lcd_dev   *pLCD                = NULL;
    ENCODED_KEY_PRESS          *pEncodedKeyPress    = NULL;
    KEY_POSITION                keyPosition         = KEY_POSITION_UP;
    INT8U                       queueError          = OS_NO_ERR;
    char                        pBarcodeMessage[LCD_WIDTH];

    memset(pBarcodeMessage, 0, LCD_WIDTH);

    // LCD setup
    if ((pLCD = alt_up_character_lcd_open_dev(CHARACTER_LCD_NAME)) == NULL)
    {
        printf("LCD setup failed\n");
    }
    else
    {
        // Clear LCD
        alt_up_character_lcd_init(pLCD);
        alt_up_character_lcd_set_cursor_pos(pLCD, 0, 0);
    }

    // Functional task loop
    while (1)
    {
        // Receive encoded keypress from ISR
        pEncodedKeyPress = (ENCODED_KEY_PRESS *)OSQPend(pBarcodeKeyPressQueue, 0, &queueError);
        if ((queueError == OS_NO_ERR) && (pEncodedKeyPress != NULL))
        {
            // Begin adding decoded keypresses to our barcode message
            if (decodeBarcode(pBarcodeMessage, pEncodedKeyPress, &keyPosition) == DECODE_STATUS_COMPLETE)
            {
                // Clear LCD
                alt_up_character_lcd_init(pLCD);
                alt_up_character_lcd_set_cursor_pos(pLCD, 0, 0);

                // Write barcode to LCD
                alt_up_character_lcd_string(pLCD, pBarcodeMessage);

                // Clear barcode message
                memset(pBarcodeMessage, 0, LCD_WIDTH);
            }

            // De-allocate encoded keypress passed from ISR
            free(pEncodedKeyPress);
            pEncodedKeyPress = NULL;
        }
        else
        {
            printf("Error pending on pBarcodeKeyPressQueue: %d\n", queueError);
        }
    }
}

/* Interrupts */
static void
BarcodeISR(void *pContext)
{
    alt_up_ps2_dev     *pBarcodeScanner = (alt_up_ps2_dev *)pContext;
    ENCODED_KEY_PRESS  *pData           = NULL;
    KB_CODE_TYPE        decodeMode      = KB_INVALID_CODE;
    alt_u8              encodedValue    = 0;
    char                asciiValue      = '\0';

    // Read byte from device
    // Assuming that decode_scancode is clearing the interupt
    if (decode_scancode(pBarcodeScanner, &decodeMode, &encodedValue, &asciiValue) == 0)
    {
        pData = (ENCODED_KEY_PRESS *)malloc(sizeof(ENCODED_KEY_PRESS));
        if (pData != NULL)
        {
            pData->decodeMode   = decodeMode;
            pData->encodedValue = encodedValue;
            OSQPost(pBarcodeKeyPressQueue, pData);
        }
    }

    return;
}

/* Entry point, setup */
int
main(void)
{
    alt_up_ps2_dev *pBarcodeScanner = NULL;
    INT8U           status          = OS_NO_ERR;

    // Setup barcode scanner
    if ((pBarcodeScanner = alt_up_ps2_open_dev(BARCODE_SCANNER_PS2_NAME)) != NULL)
    {
        pBarcodeScanner->base = BARCODE_SCANNER_PS2_BASE;
        pBarcodeScanner->irq_id = BARCODE_SCANNER_PS2_IRQ;
        pBarcodeScanner->timeout = 0;
        pBarcodeScanner->device_type = PS2_KEYBOARD;

        set_keyboard_rate(pBarcodeScanner, 0);

        alt_up_ps2_enable_read_interrupt(pBarcodeScanner);

        status = alt_irq_register(pBarcodeScanner->irq_id,
                                  (void *)pBarcodeScanner,
                                  BarcodeISR);

        if (status != OS_NO_ERR)
        {
            printf("OneHzTimerISR setup failed.\n");
        }
    }
    else
    {
        printf("Barcode scanner setup failed.\n");
    }

    // Initialize synchronized data structures
    pBarcodeKeyPressQueue = OSQCreate(pBarcodeKeyPressQueueData, LCD_MESSAGE_QUEUE_SIZE);
    if (pBarcodeKeyPressQueue == NULL)
    {
        printf("pBarcodeKeyPressQueue setup failed.\n");
    }

    // Create and initialize LCDTask
    status = OSTaskCreateExt(LCDTask,
                             NULL,
                             (void *)&LCDTaskStack[TASK_STACKSIZE-1],
                             LCD_TASK_PRIORITY,
                             LCD_TASK_PRIORITY,
                             LCDTaskStack,
                             TASK_STACKSIZE,
                             NULL,
                             0);
    if (status != OS_NO_ERR)
    {
        printf("LCDTask setup failed.\n");
    }

    // Start OS
    OSStart();

    return 0;
}

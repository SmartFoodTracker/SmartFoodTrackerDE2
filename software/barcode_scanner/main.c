#include "includes.h"
#include "barcode_scanner.h"
#include "altera_up_avalon_character_lcd.h"

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

/* LCD update task, pends on a barcodeScannerDecode and writes barcode to LCD display */
void
LCDTask(void* pData)
{
    alt_up_character_lcd_dev   *pLCD                = NULL;
    BarcodeScanner             *pBarcodeScanner     = (BarcodeScanner *)pData;
    Barcode                     barcode;

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
        alt_up_character_lcd_string(pLCD, "setup");
    }

    // Functional task loop
    while (1)
    {
        if (pBarcodeScanner != NULL)
        {
            barcodeScannerDecode(pBarcodeScanner, &barcode);

            // Clear LCD
            alt_up_character_lcd_init(pLCD);
            alt_up_character_lcd_set_cursor_pos(pLCD, 0, 0);

            // Write barcode to LCD
            alt_up_character_lcd_string(pLCD, barcode.pString);
        }
    }
}

/* Entry point, setup */
int
main(void)
{
    BarcodeScanner *pBarcodeScanner = NULL;
    INT8U           status          = OS_NO_ERR;

    // Create and initialize barcode scanner
    pBarcodeScanner = barcodeScannerCreate();

    if (pBarcodeScanner != NULL)
    {
        // Create and initialize LCDTask
        status = OSTaskCreateExt(LCDTask,
                                 (void *)pBarcodeScanner,
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
    }

    // Start OS
    OSStart();

    return 0;
}

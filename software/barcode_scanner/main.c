/** @file   main.c
 *  @brief  Testing program for PS/2 barcode scanner
 *
 *  Setup, and consumption of data produced by a BarcodeScanner object.
 *  Scanned barcodes are printed to the embedded LCD screen.
 *
 *  @author Kyle O'Shaughnessy (koshaugh)
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#include <stdio.h>
#include "includes.h"
#include "barcode_scanner.h"
#include "altera_up_avalon_character_lcd.h"

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

#define TASK_STACKSIZE      2048
#define LCD_TASK_PRIORITY   1

/*****************************************************************************/
/* Globals                                                                   */
/*****************************************************************************/

OS_STK  LCDTaskStack[TASK_STACKSIZE];

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

/**
 * @brief      LCD update task; pends on barcodeScannerDecode and writes
 *             barcode to LCD display.
 *
 * @param[in]  pData  pointer to a BarcodeScanner object pased in as task
 *                    context
 */
void
LCDTask(void* pData)
{
    alt_up_character_lcd_dev   *pLCD            = NULL;
    BarcodeScanner             *pBarcodeScanner = (BarcodeScanner *) pData;
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
    while (pBarcodeScanner != NULL)
    {
        // Wait for next decode to finish
        barcodeScannerDecode(pBarcodeScanner, &barcode);

        // Clear LCD
        alt_up_character_lcd_init(pLCD);
        alt_up_character_lcd_set_cursor_pos(pLCD, 0, 0);

        // Write barcode to LCD
        alt_up_character_lcd_string(pLCD, barcode.pString);
    }
} // LCDTask

/*****************************************************************************/

/**
 * @brief      Program entry point; initializes tasks and shared objects.
 *
 * @return     Always 0
 */
int
main(void)
{
    BarcodeScanner *pBarcodeScanner = NULL;
    INT8U           status          = OS_NO_ERR;

    // Create and initialize barcode scanner
    pBarcodeScanner = barcodeScannerCreate(BARCODE_SCANNER_PS2_NAME,
                                           BARCODE_SCANNER_PS2_BASE,
                                           BARCODE_SCANNER_PS2_IRQ);
    if (pBarcodeScanner == NULL)
    {
        printf("Barcode scanner setup failed.\n");
    }

    // Create and initialize LCDTask
    status = OSTaskCreateExt(LCDTask,
                             pBarcodeScanner,
                             &LCDTaskStack[TASK_STACKSIZE-1],
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
} // main

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

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
#include "buttons.h"
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
 * @brief      LCD update task; Enables buttons, pends on
 *             buttonsGetButtonPress, and then writes button ID (as an int) to
 *             embedded LCD
 *
 * @param[in]  pData  Pointer to a Buttons object, pased in during setup as
 *             task context
 */
void
LCDTask(void* pData)
{
    alt_up_character_lcd_dev *pLCD      = NULL;
    Buttons                  *pButtons  = (Buttons *) pData;
    Button                    button    = ButtonMax;
    char                      pButtonString[16];

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

    // Enable all buttons
    buttonsEnableButton(pButtons, ButtonAdd);
    buttonsEnableButton(pButtons, ButtonCancel);
    buttonsEnableButton(pButtons, ButtonRemove);

    // Functional task loop
    while (pButtons != NULL)
    {
        // Wait for next button press
    	button = buttonsGetButtonPress(pButtons);

    	// Write button ID to string
    	snprintf(pButtonString, 16, "Button: %d", (int) button);

        // Clear LCD
        alt_up_character_lcd_init(pLCD);
        alt_up_character_lcd_set_cursor_pos(pLCD, 0, 0);

        // Write button string to LCD
        alt_up_character_lcd_string(pLCD, pButtonString);
    }
} // LCDTask

/*****************************************************************************/

/**
 * @brief      Program entry point; Initialize tasks and shared objects.
 *
 * @return     Always 0
 */
int
main(void)
{
    Buttons    *pButtons    = NULL;
    INT8U       status      = OS_NO_ERR;

    // Create Buttons object
    pButtons = buttonsCreate();
    if (pButtons == NULL)
    {
        printf("Buttons creation failed.\n");
    }

    // TODO: Check the status of these init calls
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

    // Create and initialize LCDTask
    status = OSTaskCreateExt(LCDTask,
                             pButtons,
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

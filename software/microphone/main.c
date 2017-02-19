/** @file   main.c
 *  @brief  Testing program for push-to-talk microphone
 *
 *  @author Kyle O'Shaughnessy (koshaugh)
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#include <stdio.h>
#include "includes.h"
#include "microphone.h"
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
 * @brief       LCD update task
 * @details
 *
 * @param pData
 */
void
LCDTask(void* pData)
{
    alt_up_character_lcd_dev   *pLCD = NULL;
    Microphone                 *pMicrophone = (Microphone *) pData;

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
        alt_up_character_lcd_set_cursor_pos(pLCD, 0, 0);
        alt_up_character_lcd_string(pLCD, "hello");
    	OSTimeDlyHMSM(0, 0, 2, 0);
        alt_up_character_lcd_set_cursor_pos(pLCD, 0, 0);
        alt_up_character_lcd_string(pLCD, "HELLO");
        OSTimeDlyHMSM(0, 0, 2, 0);
    }
} // LCDTask

/*****************************************************************************/

/**
 * @brief   Program entry point
 * @details Initialize tasks and shared objects
 */
int
main(void)
{
    INT8U       status      = OS_NO_ERR;
    Microphone *pMicrophone = NULL;

    // Setup push-to-talk microphone
    pMicrophone = microphoneCreate();
    // TODO : add a null check

    // Create and initialize LCDTask
    status = OSTaskCreateExt(LCDTask,
                             pMicrophone,
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

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

#define MICROPHONE_TESTING

/*****************************************************************************/
/* Globals                                                                   */
/*****************************************************************************/

OS_STK  LCDTaskStack[TASK_STACKSIZE];

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

/**
 * @brief      LCD update task; writes status of push-to-talk microphone to LCD
 *             Display.
 *
 * @param      pData  Microphone pointer wrapped as task context
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
    while (pMicrophone != NULL)
    {
        // Wait for push-to-talk switch to trigger recording
        microphoneWaitAndBeginRecording(pMicrophone);
        alt_up_character_lcd_init(pLCD);
        alt_up_character_lcd_set_cursor_pos(pLCD, 0, 0);
        alt_up_character_lcd_string(pLCD, "Begin");

        // Wait for push-to-talk switch to trigger record completion
        microphoneFinishRecording(pMicrophone);
        alt_up_character_lcd_init(pLCD);
        alt_up_character_lcd_set_cursor_pos(pLCD, 0, 0);
        alt_up_character_lcd_string(pLCD, "Finish");

#ifdef TODO_EXTRA_FUNCTIONS
        // Playback the recorded buffer to LINE_OUT
        microphonePlayback(pMicrophone);
#endif // TODO_EXTRA_FUNCTIONS
    }

    // Test loop, reads a word from read fifo and writes it back to the write fifo
//    while (1)
//    {
//        int fifospace = alt_up_audio_read_fifo_avail (pMicrophone->pHandle, ALT_UP_AUDIO_RIGHT);
//        unsigned int l_buf;
//        unsigned int r_buf;
//        if ( fifospace > 0 ) // check if data is available
//        {
//            // read audio buffer
//            alt_up_audio_read_fifo (pMicrophone->pHandle, &(r_buf), 1, ALT_UP_AUDIO_RIGHT);
//            alt_up_audio_read_fifo (pMicrophone->pHandle, &(l_buf), 1, ALT_UP_AUDIO_LEFT);
//            // write audio buffer
//            alt_up_audio_write_fifo (pMicrophone->pHandle, &(r_buf), 1, ALT_UP_AUDIO_RIGHT);
//            alt_up_audio_write_fifo (pMicrophone->pHandle, &(l_buf), 1, ALT_UP_AUDIO_LEFT);
//        }
//    }
} // LCDTask

/*****************************************************************************/

/**
 * @brief      Program entry point; initialize tasks and shared objects.
 *
 * @return     Always 0
 */
int
main(void)
{
    INT8U       status      = OS_NO_ERR;
    Microphone *pMicrophone = NULL;

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

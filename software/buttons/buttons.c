/** @file   buttons.c
 *  @brief  Source for public and private routines used for the onboard buttons
 *
 *  Driver for the DE2 onboard buttons. All non-static functions are
 *  effectively part of the public API, those that are static should be
 *  considered private.
 *
 *  @author Kyle O'Shaughnessy (koshaugh)
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "altera_avalon_pio_regs.h"
#include "buttons.h"

/*****************************************************************************/
/* Declarations                                                              */
/*****************************************************************************/

static Buttons* acquireButtons();
static void     releaseButtons(Buttons *pButtons);
static INT8U    initQueue(Buttons *pButtons);
static void     buttonISR(void *pContext, alt_u32 id);

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

/**
 * @brief      Allocates a Buttons object on the heap and initializes it.
 *             Caller must call buttonsInitButton(), and buttonsEnableButton()
 *             after this routine has been run for each new button. To clean
 *             up the object, call buttonsDestroy().
 *
 * @return     A new Buttons object
 */
Buttons*
buttonsCreate()
{
    INT8U       status      = OS_NO_ERR;
    Buttons    *pButtons    = acquireButtons();

    if (pButtons == NULL)
    {
        status = OS_ERR_PDATA_NULL;
    }

    if (status == OS_NO_ERR)
    {
        status = initQueue(pButtons);
    }

    if (status != OS_NO_ERR)
    {
        buttonsDestroy(pButtons);
        pButtons = NULL;
    }

    return pButtons;
} // buttonsCreate

/*****************************************************************************/

/**
 * @brief      Uninitialize and cleanup a Buttons object. This should be called
 *             if buttonsCreate() was called to create the object.
 *
 * @param[in]  pButtons  Pointer to Buttons object to be cleaned-up.
 */
void
buttonsDestroy(Buttons *pButtons)
{
    releaseButtons(pButtons);
} // buttonsDestroy

/*****************************************************************************/

/**
 * @brief      Add a new button to the Buttons object. This disables the
 *             button's interrupt by default and registers it's ISR.
 *
 * @param[in]  pButtons     Valid handle for Buttons object
 * @param[in]  buttonID     Button id
 * @param[in]  baseAddress  Button base address
 * @param[in]  irq          Button IRQ number
 *
 * @return     OS_NO_ERR if no error, otherwise OS_ERR_PDATA_NULL if ISR
 *             registration fails or invalid parameters
 */
INT8U
buttonsInitButton(Buttons      *pButtons,
                  Button        buttonID,
                  unsigned int  baseAddress,
                  unsigned int  irq)
{
    INT8U           status      = OS_NO_ERR;
    ButtonContext  *pIsrContext = NULL;

    if (pButtons && (buttonID < ButtonMax))
    {
        pIsrContext = (ButtonContext *) malloc(sizeof(ButtonContext));
        if (pIsrContext)
        {
            // Fill out ISR context
            pIsrContext->pButtons       = pButtons;
            pIsrContext->irq            = irq;
            pIsrContext->baseAddress    = baseAddress;
            pIsrContext->buttonID       = buttonID;

            // Disable and and free buttons that are being re-initialized
            if (pButtons->isrContexts[buttonID])
            {
                buttonsDisableButton(pButtons, buttonID);
                free(pButtons->isrContexts[buttonID]);
                pButtons->isrContexts[buttonID] = NULL;
            }

            // Save context for later cleanup
            pButtons->isrContexts[buttonID] = pIsrContext;

            // Disable button by default
            buttonsDisableButton(pButtons, buttonID);

            // Register ISR and pass context
            alt_irq_register(irq, pIsrContext, buttonISR);

        }
        else
        {
            pButtons->isrContexts[buttonID] = NULL;
            status = OS_ERR_PDATA_NULL;
        }
    }
    else
    {
        status = OS_ERR_PDATA_NULL;
    }

    return status;
} // buttonsInitButton

/*****************************************************************************/

/**
 * @brief      Enables interrupt for a button.
 *
 * @param[in]  pButtons  Valid handle for Buttons object
 * @param[in]  buttonID  Button id
 */
void
buttonsEnableButton(Buttons *pButtons, Button buttonID)
{
    unsigned int baseAddress = 0;
    if (pButtons && (buttonID < ButtonMax))
    {
        if (pButtons->isrContexts[buttonID])
        {
            baseAddress = pButtons->isrContexts[buttonID]->baseAddress;
            IOWR_ALTERA_AVALON_PIO_IRQ_MASK(baseAddress, 0xf);
            IOWR_ALTERA_AVALON_PIO_EDGE_CAP(baseAddress, 0x0);
        }
    }
} // buttonsEnableButton

/*****************************************************************************/

/**
 * @brief      Disables interrupt for a button.
 *
 * @param[in]  pButtons  Valid handle for Buttons object
 * @param[in]  buttonID  Button id
 */
void
buttonsDisableButton(Buttons *pButtons, Button buttonID)
{
    unsigned int baseAddress = 0;
    if (pButtons && (buttonID < ButtonMax))
    {
        if (pButtons->isrContexts[buttonID])
        {
            baseAddress = pButtons->isrContexts[buttonID]->baseAddress;
            IOWR_ALTERA_AVALON_PIO_IRQ_MASK(baseAddress, 0x0);
            IOWR_ALTERA_AVALON_PIO_EDGE_CAP(baseAddress, 0x0);
        }
    }
} // buttonsDisableButton

/*****************************************************************************/

/**
 * @brief      Enable interrupts for all buttons.
 *
 * @param[in]  pButtons  Valid handle for Buttons object
 */
void
buttonsEnableAll(Buttons *pButtons)
{
	Button button;
	if (pButtons)
	{
		for (button = 0; button < ButtonMax; button++)
		{
			buttonsEnableButton(pButtons, button);
		}
	}
} // buttonsEnableAll

/*****************************************************************************/

/**
 * @brief      Disable interrupts for all buttons.
 *
 * @param[in]  pButtons  Valid handle for Buttons object
 */
void
buttonsDisableAll(Buttons *pButtons)
{
	Button button;
	if (pButtons)
	{
		for (button = 0; button < ButtonMax; button++)
		{
			buttonsDisableButton(pButtons, button);
		}
	}
} // buttonsDisableAll

/*****************************************************************************/

/**
 * @brief      Wait for next button press and get it's id.
 *
 * @param[in]  pButtons  Valid handle for Buttons object
 *
 * @return     Button id of next available button press
 */
Button
buttonsGetButtonPress(Buttons *pButtons)
{
    INT8U   queueError  = OS_NO_ERR;
    Button  buttonID    = ButtonMax;

    if (pButtons && pButtons->pButtonPressQueue)
    {
        buttonID = (Button) OSQPend(pButtons->pButtonPressQueue,
                                    0,
                                    &queueError);
        if (queueError != OS_NO_ERR)
        {
            buttonID = ButtonMax;
        }
    }

    return buttonID;
} // buttonsGetButtonPress

/*****************************************************************************/
/* Static Functions                                                          */
/*****************************************************************************/

/**
 * @brief      Heap allocate and initialize members of a Buttons object.
 *             Cleanup the object later using releaseButtons().
 *
 * @return     A new buttons object or NULL if malloc failed
 */
static Buttons*
acquireButtons()
{
    Button      button   = 0;
    Buttons    *pButtons = (Buttons *) malloc(sizeof(Buttons));

    if (pButtons)
    {
        for (button = 0; button < ButtonMax; button++)
        {
            pButtons->isrContexts[button] = NULL;
        }

        pButtons->pButtonPressQueue = NULL;
    }

    return pButtons;
} // acquireButtons

/*****************************************************************************/

/**
 * @brief      Release all resources associated with a Buttons object including
 *             itself. This will disable associated interrupts, release system
 *             queues, and free the ISR contexts.
 *
 * @param[in]  pButtons  Valid handle for Buttons object
 */
static void
releaseButtons(Buttons *pButtons)
{
    Button  button      = 0;
    INT8U   queueError  = OS_NO_ERR;

    if (pButtons)
    {
        // Cleanup ISR contexts
        for (button = 0; button < ButtonMax; button++)
        {
            if (pButtons->isrContexts[button])
            {
                // Disable interrupts
                buttonsDisableButton(pButtons, button);

                // Free heap allocated memory
                free(pButtons->isrContexts[button]);
                pButtons->isrContexts[button] = NULL;
            }
        }

        // Cleanup queue
        if(pButtons->pButtonPressQueue)
        {
            OSQDel(pButtons->pButtonPressQueue,
                   OS_DEL_ALWAYS,
                   &queueError);

            pButtons->pButtonPressQueue = NULL;
        }

        // Free heap allocated memory
        free(pButtons);
    }
} // releaseButtons

/*****************************************************************************/

/**
 * @brief      Initialize the button press queue.
 *
 * @param[in]  pButtons  Valid handle for Buttons object
 *
 * @return     OS_NO_ERR if no error, OS_ERR_PDATA_NULL is OSQCreate fails or
 *             if parameters are invalid
 */
static INT8U
initQueue(Buttons *pButtons)
{
    INT8U status = OS_NO_ERR;

    if (pButtons)
    {
        pButtons->pButtonPressQueue = OSQCreate(pButtons->pButtonPressQueueData,
                                                BUTTONS_MESSAGE_QUEUE_SIZE);
        if (pButtons->pButtonPressQueue == NULL)
        {
            status = OS_ERR_PDATA_NULL;
        }
    }
    else
    {
        status = OS_ERR_PDATA_NULL;
    }

    return status;
} // initQueue

/*****************************************************************************/

/**
 * @brief      Interrupt service routine for all button presses. This routine
 *             is triggered on the falling edge of the registered buttons.
 *             The button id of the pressed button is posted to the shared
 *             queue.
 *
 * @param[in]  pContext  ButtonContext handle wrapped as an isr context.
 * @param[in]  id        UNUSED_PARAMETER
 */
static void
buttonISR(void *pContext, alt_u32 id)
{
    ButtonContext *pButtonContext = (ButtonContext *) pContext;

    // Push buttonID onto the message queue
    OSQPost(pButtonContext->pButtons->pButtonPressQueue, (void *) pButtonContext->buttonID);

    // Reset the button's edge capture register
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(pButtonContext->baseAddress, 0x0);

    // Read the value to delay ISR exit
    // This helps with ignoring spurious interupts
    IORD_ALTERA_AVALON_PIO_EDGE_CAP(pButtonContext->baseAddress);

} // buttonISR

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

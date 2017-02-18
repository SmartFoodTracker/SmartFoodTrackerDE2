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

void
buttonsDestroy(Buttons *pButtons)
{
    releaseButtons(pButtons);
} // buttonsDestroy

/*****************************************************************************/

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

            // Save context for later
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

Button
buttonsGetButtonPress(Buttons *pButtons)
{
    INT8U   queueError  = OS_NO_ERR;
    Button  buttonID    = ButtonMax;

    if (pButtons)
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
                // Disable the button's interrupt
                IOWR_ALTERA_AVALON_PIO_IRQ_MASK(pButtons->isrContexts[button]->baseAddress, 0xf);

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

static void
buttonISR(void *pContext, alt_u32 id)
{
    ButtonContext *pButtonContext = (ButtonContext *) pContext;

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

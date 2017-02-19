/** @file   microphone.c
 *  @brief  Source for public and private routines used for the push-to-talk
 *          microphone.
 *
 *  Driver for the DE2 microphone and "push-to-talk" switch. All non-static
 *  functions are effectively part of the public API, those that are static
 *  should be considered private.
 *
 *  @author Kyle O'Shaughnessy (koshaugh)
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "altera_avalon_pio_regs.h"
#include "microphone.h"

/*****************************************************************************/
/* Declarations                                                              */
/*****************************************************************************/

static Microphone*  acquireMicrophone();
static void         releaseMicrophone(Microphone *pMicrophone);
static INT8U        initSemaphore(Microphone *pMicrophone);
static INT8U        initSwitch(Microphone   *pMicrophone,
                               unsigned int  switchBaseAddress,
                               unsigned int  switchIRQ);
static void         switchISR(void *pContext, alt_u32 id);

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

Microphone*
microphoneCreate(unsigned int switchBaseAddress,
                 unsigned int switchIRQ)
{
    INT8U       status      = OS_NO_ERR;
    Microphone *pMicrophone = acquireMicrophone();

    if (pMicrophone == NULL)
    {
        status = OS_ERR_PDATA_NULL;
    }

    // Initialize the "push-to-talk" semaphore
    if (status == OS_NO_ERR)
    {
        status = initSemaphore(pMicrophone);
    }

    // Enable switch interrupt
    if (status == OS_NO_ERR)
    {
        status = initSwitch(pMicrophone, switchBaseAddress, switchIRQ);
    }

    // TODO: setup microphone and audio codec

    if (status != OS_NO_ERR)
    {
        microphoneDestroy(pMicrophone);
        pMicrophone = NULL;
    }

    return pMicrophone;
} // microphoneCreate

/*****************************************************************************/

void
microphoneDestroy(Microphone *pMicrophone)
{
    releaseMicrophone(pMicrophone);
} // microphoneDestroy

/*****************************************************************************/

void
microphoneWaitAndBeginRecording(Microphone *pMicrophone)
{
    INT8U semError = OS_NO_ERR;
    if (pMicrophone)
    {
        OSSemPend(pMicrophone->pPushToTalkSemaphore, 0, &semError);
    }
    // TODO: begin recording on audio codec
} // microphoneWaitAndBeginRecording

/*****************************************************************************/

void
microphoneFinishRecording(Microphone *pMicrophone)
{
    INT8U semError = OS_NO_ERR;
    if (pMicrophone)
    {
        // TODO: add a proper timeout
        OSSemPend(pMicrophone->pPushToTalkSemaphore, 0, &semError);
    }
    // TODO: complete recording on audio codec
} // microphoneFinishRecording

/*****************************************************************************/
/* Static Functions                                                          */
/*****************************************************************************/

static Microphone*
acquireMicrophone()
{
    Microphone *pMicrophone = (Microphone *) malloc(sizeof(Microphone));
    
    if (pMicrophone)
    {
        pMicrophone->switchBaseAddress      = 0;
        pMicrophone->pPushToTalkSemaphore   = NULL;
    }

    return pMicrophone;
} // acquireMicrophone

/*****************************************************************************/

static void
releaseMicrophone(Microphone *pMicrophone)
{
    INT8U semError = OS_NO_ERR;
    if (pMicrophone)
    {
        // Disable switch interrupt
        if (pMicrophone->switchBaseAddress)
        {
            IOWR_ALTERA_AVALON_PIO_IRQ_MASK(pMicrophone->switchBaseAddress, 0x0);
            IOWR_ALTERA_AVALON_PIO_EDGE_CAP(pMicrophone->switchBaseAddress, 0x0);
        }

        // Cleanup semaphore
        if (pMicrophone->pPushToTalkSemaphore)
        {
            OSSemDel(pMicrophone->pPushToTalkSemaphore,
                     OS_DEL_ALWAYS,
                     &semError);
            pMicrophone->pPushToTalkSemaphore = NULL;
        }

        free(pMicrophone);
    }
} // releaseMicrophone

/*****************************************************************************/

static INT8U
initSemaphore(Microphone *pMicrophone)
{
    INT8U status = OS_NO_ERR;

    if (pMicrophone && (pMicrophone->pPushToTalkSemaphore == NULL))
    {
        pMicrophone->pPushToTalkSemaphore = OSSemCreate(0);
        if (pMicrophone->pPushToTalkSemaphore == NULL)
        {
            status = OS_ERR_PDATA_NULL;
        }
    }
    else
    {
        status = OS_ERR_PDATA_NULL;
    }

    return status;
} // initSemaphore

/*****************************************************************************/

static INT8U
initSwitch(Microphone   *pMicrophone,
           unsigned int  switchBaseAddress,
           unsigned int  switchIRQ)
{
    INT8U status = OS_NO_ERR;

    if (pMicrophone)
    {
        // Save base address for use inside switchISR
        pMicrophone->switchBaseAddress = switchBaseAddress;

        // Reset edge capture register and enable interrupt
        IOWR_ALTERA_AVALON_PIO_IRQ_MASK(switchBaseAddress, 0xf);
        IOWR_ALTERA_AVALON_PIO_EDGE_CAP(switchBaseAddress, 0x0);

        // Register ISR and pass microphone as context
        status = alt_irq_register(switchIRQ, pMicrophone, switchISR);
    }

    return status;
} // initSwitch

/*****************************************************************************/

static void
switchISR(void *pContext, alt_u32 id)
{
    Microphone *pMicrophone = (Microphone *) pContext;

    // We don't really care about the return value of this call
    OSSemPost(pMicrophone->pPushToTalkSemaphore);

    // Reset the button's edge capture register
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(pMicrophone->switchBaseAddress, 0x0);

    // Read the value to delay ISR exit
    // This helps with ignoring spurious interrupts
    IORD_ALTERA_AVALON_PIO_EDGE_CAP(pMicrophone->switchBaseAddress);
} // switchISR

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

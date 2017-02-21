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
#include "includes.h"
#include "altera_avalon_pio_regs.h"
#include "microphone.h"

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

/*****************************************************************************/
/* Declarations                                                              */
/*****************************************************************************/

static Microphone*  acquireMicrophone();
static void         releaseMicrophone(Microphone *pMicrophone);
static INT8U        initHandle(Microphone *pMicrophone,
                               const char *pName,
                               unsigned int audioCoreIRQ);
static INT8U        initSemaphore(Microphone *pMicrophone);
static INT8U        initSwitch(Microphone   *pMicrophone,
                               unsigned int  switchBaseAddress,
                               unsigned int  switchIRQ);
#ifdef TODO_EXTRA_FUNCTIONS
static void         clearRecording(Microphone *pMicrophone);
#endif // TODO_EXTRA_FUNCTIONS
static void         switchISR(void *pContext, alt_u32 id);
static void         codecFifoISR(void *pContext, alt_u32 id);

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

/**
 * @brief      Create and initialize a new Microphone object. This memory is
 *             allocated on the heap and should be cleaned up by the caller
 *             using microphoneDestroy(...). Interrupt service routines are
 *             registered for both the push-to-talk switch and the audio codec
 *             fifos.
 *
 * @param[in]  pName              Audio core device name; ex: /dev/audio
 * @param[in]  audioCoreIRQ       Audio core IRQ number
 * @param[in]  switchBaseAddress  Switch base address
 * @param[in]  switchIRQ          Switch IRQ number
 *
 * @return     Pointer to new Microphone object
 */
Microphone*
microphoneCreate(const char   *pName,
                 unsigned int  audioCoreIRQ,
                 unsigned int  switchBaseAddress,
                 unsigned int  switchIRQ)
{
    INT8U       status      = OS_NO_ERR;
    Microphone *pMicrophone = acquireMicrophone();

    if (pMicrophone == NULL)
    {
        status = OS_ERR_PDATA_NULL;
    }

    // Initialize audio core device handle
    if (status == OS_NO_ERR)
    {
        status  = initHandle(pMicrophone, pName, audioCoreIRQ);
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

    // Clean-up in case of error
    if (status != OS_NO_ERR)
    {
        microphoneDestroy(pMicrophone);
        pMicrophone = NULL;
    }

    return pMicrophone;
} // microphoneCreate

/*****************************************************************************/

/**
 * @brief      Release all resources held by the Microphone handle pMicrophone.
 *             Users should call this for all handles created using
 *             microphoneCreate(...). Following a call to this routine, the
 *             handle passed in should be set to NULL to avoid referencing
 *             invalid memory.
 *
 * @param[in]  pMicrophone  Handle to resources to be released
 */
void
microphoneDestroy(Microphone *pMicrophone)
{
    releaseMicrophone(pMicrophone);
} // microphoneDestroy

/*****************************************************************************/

/**
 * @brief      Wait for push-to-talk sequence to begin then start recording.
 *             This is a blocking routine due to an indefinite pend on a
 *             semaphore. Upon unblocking, the audio codec's read fifo ISR is
 *             enabled and data transfer begins. This routine should be matched
 *             with a call to microphoneFinishRecording(...) shortly afterwards.
 *
 * @param[in]  pMicrophone  Valid microphone handle
 */
void
microphoneWaitAndBeginRecording(Microphone *pMicrophone)
{
    INT8U semError = OS_NO_ERR;
    if (pMicrophone)
    {
        // Reset recording buffer and next sample pointer
        memset(pMicrophone->recordingBuffer, 0, RECORDING_BUFFER_SIZE * sizeof(unsigned int));
        pMicrophone->pNextSample = pMicrophone->recordingBuffer;

        // Wait indefinitely for next push-to-talk sequence
        OSSemPend(pMicrophone->pPushToTalkSemaphore, 0, &semError);

        // Clear codec fifos
        alt_up_audio_reset_audio_core(pMicrophone->pHandle);

        // Enable read interrupt, this will trigger recording sequence
        alt_up_audio_enable_read_interrupt(pMicrophone->pHandle);
    }

} // microphoneWaitAndBeginRecording

/*****************************************************************************/

/**
 * @brief      Wait for push-to-talk sequence to end, this is triggered by
 *             the push-to-talk switch being moved to the downward position.
 *             This is a blocking call due to a indefinite pend on a semaphore.
 *             This should only be called after a call to the
 *             microphoneWaitAndBeginRecording(...) routine is called. This
 *             routine will turn off the codec's readFifo interrupts and stop
 *             data transfer.
 *
 * @param[in]  pMicrophone  Valid microphone handle
 */
void
microphoneFinishRecording(Microphone *pMicrophone)
{
    INT8U semError = OS_NO_ERR;
    if (pMicrophone)
    {
        OSSemPend(pMicrophone->pPushToTalkSemaphore, 0, &semError);
        alt_up_audio_disable_read_interrupt(pMicrophone->pHandle);
    }
} // microphoneFinishRecording

#ifdef TODO_EXTRA_FUNCTIONS
/*****************************************************************************/

/**
 * @brief      { function_description }
 *
 * @param[in]  pMicrophone  The microphone
 */
void
microphoneEnablePushToTalk(Microphone *pMicrophone)
{

} // microphoneEnablePushToTalk

/*****************************************************************************/

/**
 * @brief      { function_description }
 *
 * @param[in]  pMicrophone  The microphone
 */
void
microphoneDisablePushToTalk(Microphone *pMicrophone)
{

} // microphoneDisablePushToTalk

/*****************************************************************************/

/**
 * @brief        { function_description }
 *
 * @param[in]    pMicrophone         The microphone
 * @param[inout] pLinear16Recording  The linear 16 recording
 */
void
microphoneExportLinear16(Microphone        *pMicrophone,
                         Linear16Recording *pLinear16Recording)
{

} // microphoneExportLinear16

/*****************************************************************************/

#ifdef MICROPHONE_TESTING
/**
 * @brief      { function_description }
 *
 * @param[in]  pMicrophone  The microphone
 */
void
microphonePlayback(Microphone *pMicrophone)
{
    // Enable codec write interrupt
} // microphonePlayback
#endif // MICROPHONE_TESTING
#endif // TODO_EXTRA_FUNCTIONS

/*****************************************************************************/
/* Static Functions                                                          */
/*****************************************************************************/

/**
 * @brief      Allocate a Microphone object on the heap and initialize all
 *             members. Object should be released with releaseMicrophone(...).
 *
 * @return     A new Microphone object
 */
static Microphone*
acquireMicrophone()
{
    Microphone *pMicrophone = (Microphone *) malloc(sizeof(Microphone));

    if (pMicrophone)
    {
        pMicrophone->switchBaseAddress      = 0;
        pMicrophone->pPushToTalkSemaphore   = NULL;
        pMicrophone->pHandle                = NULL;
        pMicrophone->pNextSample            = pMicrophone->recordingBuffer;
        memset(pMicrophone->recordingBuffer, 0, RECORDING_BUFFER_SIZE * sizeof(unsigned int));
    }

    return pMicrophone;
} // acquireMicrophone

/*****************************************************************************/

/**
 * @brief      Frees all resources associated with a Microphone object and the
 *             object itself. All associated interrupts are disabled. System
 *             resources are deleted and returned to their pools. Any
 *             Microphone allocated with acquireMicrophone should be cleaned up
 *             using this routine.
 *
 * @param[in]  pMicrophone  Microphone object to be released
 */
static void
releaseMicrophone(Microphone *pMicrophone)
{
    INT8U semError = OS_NO_ERR;
    if (pMicrophone)
    {
        // Disable audio core interrupt
        if (pMicrophone->pHandle)
        {
            alt_up_audio_disable_read_interrupt(pMicrophone->pHandle);
            alt_up_audio_disable_write_interrupt(pMicrophone->pHandle);
            pMicrophone->pHandle = NULL;
        }

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

/**
 * @brief      Initialize handle for audio core device. Disables interrupts and
 *             registers the readFifo ISR.
 *
 * @param[in]  pMicrophone   Valid microphone handle
 * @param[in]  pName         Audio core device name; ex: /dev/audio
 * @param[in]  audioCoreIRQ  Audio core IRQ number
 *
 * @return     OS_NO_ERR if no error, OS_ERR_PDATA_NULL if device not found,
 *             negative if ISR failed to register
 */
static INT8U
initHandle(Microphone *pMicrophone, const char *pName, unsigned int audioCoreIRQ)
{
    INT8U status = OS_NO_ERR;

    if (pMicrophone)
    {
        pMicrophone->pHandle = alt_up_audio_open_dev(pName);

        if (pMicrophone->pHandle == NULL)
        {
            status = OS_ERR_PDATA_NULL;
        }
        else
        {
            alt_up_audio_disable_read_interrupt(pMicrophone->pHandle);
            alt_up_audio_disable_write_interrupt(pMicrophone->pHandle);
            alt_up_audio_reset_audio_core(pMicrophone->pHandle);
            status = alt_irq_register(audioCoreIRQ, pMicrophone, codecFifoISR);
        }
    }

    return status;
} // initHandle

/*****************************************************************************/

/**
 * @brief      Initialize push-to-talk semaphore.
 *
 * @param[in]  pMicrophone  Valid microphone handle
 *
 * @return     OS_NO_ERR if no error, OS_ERR_PDATA_NULL if semaphore init
 *             failed or if microphone handle is invalid
 */
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

/**
 * @brief      Initialize ISR for push-to-talk switch and enable the interrupt.
 *
 * @param[in]  pMicrophone        Valid microphone handle
 * @param[in]  switchBaseAddress  Switch base address
 * @param[in]  switchIRQ          Switch IRQ number
 *
 * @return     { description_of_the_return_value }
 */
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

#ifdef TODO_EXTRA_FUNCTIONS
/**
 * @brief      { function_description }
 *
 * @param[in]  pMicrophone  The microphone
 */
static void
clearRecording(Microphone *pMicrophone)
{

} // clearRecording
#endif // TODO_EXTRA_FUNCTIONS

/*****************************************************************************/

/**
 * @brief      Interrupt service routine for the push-to-talk switch. Simply
 *             posts to the push-to-talk semaphore pended on by
 *             microphoneFinishRecording and microphoneWaitAndBeginRecording.
 *             This is an ALL edge sensitive ISR.
 *
 * @param[in]  pContext  Microphone handle wrapped as an ISR context
 * @param[in]  id        UNUSED PARAMETER
 */
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

/**
 * @brief      Interrupt service routine for the audio codec read fifo. This
 *             ISR is triggered when data is available for read from the
 *             channel fifos. This routine simply copies available data to
 *             the microphone's recording buffer. This isr is enabled/disabled
 *             by microphoneWaitAndBeginRecording/microphoneFinishRecording.
 *
 * @param      pContext  Microphone handle wrapped as an ISR context
 * @param[in]  id        UNUSED PARAMETER
 */
static void
codecFifoISR(void *pContext, alt_u32 id)
{
    Microphone     *pMicrophone         = (Microphone *) pContext;
    unsigned int    wordsRead           = 0;
    unsigned int    wordsToRead         = 0;
    unsigned int    remainingBufferSize = 0;

    if (alt_up_audio_read_interrupt_pending(pMicrophone->pHandle) == 1)
    {
        // Calculate the number of words to read
        remainingBufferSize = RECORDING_BUFFER_SIZE - (pMicrophone->pNextSample - pMicrophone->recordingBuffer);
        wordsToRead = alt_up_audio_read_fifo_avail(pMicrophone->pHandle,
                                                   ALT_UP_AUDIO_LEFT);
        wordsToRead = min(wordsToRead, remainingBufferSize);

        // Read `wordsToRead` words from the specified channel
        if (wordsToRead > 0)
        {
            // Read both channels to advance both fifos
            wordsRead = alt_up_audio_read_fifo(pMicrophone->pHandle,
                                               pMicrophone->pNextSample,
                                               wordsToRead,
                                               ALT_UP_AUDIO_RIGHT);

            wordsRead = alt_up_audio_read_fifo(pMicrophone->pHandle,
                                               pMicrophone->pNextSample,
                                               wordsToRead,
                                               ALT_UP_AUDIO_LEFT);
            pMicrophone->pNextSample += wordsRead;
        }
        else
        {
            // Buffer is full, disable reading, push-to-talk sequence is done
            alt_up_audio_disable_read_interrupt(pMicrophone->pHandle);
            OSSemPost(pMicrophone->pPushToTalkSemaphore);
        }
    }
#ifdef TODO_EXTRA_FUNCTIONS
#ifdef MICROPHONE_TESTING
    if (alt_up_audio_write_interrupt_pending(pMicrophone->pHandle) == 1)
    {
        // Copy pMicrophone->recordingBuffer to the write fifo in chunks
        // Disable write interrupt when recoring has finished being copied
    }
#endif // MICROPHONE_TESTING
#endif // TODO_EXTRA_FUNCTIONS
} // codecFifoISR

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

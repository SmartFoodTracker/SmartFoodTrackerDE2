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
static void         clearRecording(Microphone *pMicrophone);
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
        // Wait indefinitely for next push-to-talk sequence
        OSSemPend(pMicrophone->pPushToTalkSemaphore, 0, &semError);

        // Reset recording
        clearRecording(pMicrophone);

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
microphoneWaitAndFinishRecording(Microphone *pMicrophone)
{
    INT8U semError = OS_NO_ERR;
    if (pMicrophone)
    {
        OSSemPend(pMicrophone->pPushToTalkSemaphore, 0, &semError);
        alt_up_audio_disable_read_interrupt(pMicrophone->pHandle);
    }
} // microphoneFinishRecording

/*****************************************************************************/

/**
 * @brief      Simply enables the switch's interrupts, this will allow the
 *             push-to-talk sequence to start.
 *
 * @param[in]  pMicrophone  Valid microphone handle
 */
void
microphoneEnablePushToTalk(Microphone *pMicrophone)
{
    if (pMicrophone)
    {
        alt_irq_enable(pMicrophone->switchIRQ);
    }
} // microphoneEnablePushToTalk

/*****************************************************************************/

/**
 * @brief      Simply disables the switch's interrupts, this will prevent the
 *             push-to-talk sequence from starting.
 *
 * @param[in]  pMicrophone  Valid microphone handle
 */
void
microphoneDisablePushToTalk(Microphone *pMicrophone)
{
    if (pMicrophone)
    {
        alt_irq_disable(pMicrophone->switchIRQ);
    }
} // microphoneDisablePushToTalk

/*****************************************************************************/

/**
 * @brief        Moves the two MSBs from each 32-bit sample into 2-byte shorts.
 *               This data format is required for the Google Speech API. Also
 *               the buffer should be Little Endian byte order.
 *
 * @param[in]    pMicrophone         Valid microphone handle
 * @param[inout] pLinear16Recording  Buffer to be filled with 16-bit samples
 */
void
microphoneExportLinear16(Microphone        *pMicrophone,
                         Linear16Recording *pLinear16Recording)
{
    unsigned int    sample      = 0;
    unsigned short  dataPoint   = 0;
    if (pMicrophone && pLinear16Recording)
    {
        if (pMicrophone->totalSamples > 0)
        {
            for (sample = 0; sample < pMicrophone->totalSamples; sample++)
            {
                dataPoint = (unsigned short) (pMicrophone->pRecordingBuffer[sample] >> 16);
                pLinear16Recording->pRecording[sample] = dataPoint;
            }
        }

        pLinear16Recording->size = pMicrophone->totalSamples;
    }
} // microphoneExportLinear16

/*****************************************************************************/

/**
 * @brief      Enable's write interrupts for the audio codec. This will trigger
 *             codecFifoISR to begin writing pMicrophone->pRecordingBuffer to
 *             the codec's write fifo. Recorded audio clip will play to
 *             LINE_OUT.
 *
 * @param[in]  pMicrophone  Valid microphone handle
 */
void
microphonePlaybackRecording(Microphone *pMicrophone)
{
    if (pMicrophone)
    {
        // Move the read/write pointer back to the beginning of recording buffer
        pMicrophone->pNextSample = pMicrophone->pRecordingBuffer;

        // Clear codec fifos
        alt_up_audio_reset_audio_core(pMicrophone->pHandle);

        // Turn on write interrupts, this will begin write sequence to output fifos
        alt_up_audio_enable_write_interrupt(pMicrophone->pHandle);
    }
} // microphonePlaybackRecording

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
        pMicrophone->switchIRQ              = 0;
        pMicrophone->pPushToTalkSemaphore   = NULL;
        pMicrophone->pHandle                = NULL;
        clearRecording(pMicrophone);
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
 * @return     OS_NO_ERR if no error
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
        pMicrophone->switchBaseAddress  = switchBaseAddress;

        // Save IRQ so we can enable/disable later
        pMicrophone->switchIRQ          = switchIRQ;

        // Reset edge capture register and enable interrupt
        IOWR_ALTERA_AVALON_PIO_IRQ_MASK(switchBaseAddress, 0xf);
        IOWR_ALTERA_AVALON_PIO_EDGE_CAP(switchBaseAddress, 0x0);

        // Register ISR and pass microphone as context
        status = alt_irq_register(switchIRQ, pMicrophone, switchISR);
    }

    return status;
} // initSwitch

/*****************************************************************************/

/**
 * @brief      Resets the recorded buffer, read/write pointer, and sample count.
 *
 * @param[in]  pMicrophone  Valid microphone handle
 */
static void
clearRecording(Microphone *pMicrophone)
{
    if (pMicrophone)
    {
        pMicrophone->pNextSample    = pMicrophone->pRecordingBuffer;
        pMicrophone->totalSamples   = 0;
        memset(pMicrophone->pRecordingBuffer, 0, RECORDING_BUFFER_SIZE * sizeof(unsigned int));
    }
} // clearRecording

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
 * @brief      Interrupt service routine for the audio codec read/write fifo.
 *             This ISR is triggered when data is available for read from the
 *             channel fifos or if room is available for writing. This routine
 *             simply copies available data to/from the microphone's recording
 *             buffer. This isr is enabled/disabled by
 *             microphoneWaitAndBeginRecording/microphoneFinishRecording and
 *             microphonePlaybackRecording.
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

    // Reading audio from the codec
    if (alt_up_audio_read_interrupt_pending(pMicrophone->pHandle) == 1)
    {
        // Calculate the number of words to read
        remainingBufferSize = RECORDING_BUFFER_SIZE - pMicrophone->totalSamples;
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

            // Progress the write pointer forward
            pMicrophone->pNextSample    += wordsRead;
            pMicrophone->totalSamples   += wordsRead;
        }
        else
        {
            // Buffer is full, disable reading, push-to-talk sequence is done
            alt_up_audio_disable_read_interrupt(pMicrophone->pHandle);
            OSSemPost(pMicrophone->pPushToTalkSemaphore);
        }
    }

    // Writing recorded audio to the codec
    if (alt_up_audio_write_interrupt_pending(pMicrophone->pHandle) == 1)
    {
        remainingBufferSize = pMicrophone->totalSamples -
                              (pMicrophone->pNextSample -
                               pMicrophone->pRecordingBuffer);
        // Copy recording buffer to the write fifo in chunks
        wordsToRead = alt_up_audio_write_fifo_space(pMicrophone->pHandle,
                                                    ALT_UP_AUDIO_LEFT);
        wordsToRead = min(wordsToRead, remainingBufferSize);

        if (wordsToRead > 0)
        {
            // Write the recorded audio to both channels
            wordsRead = alt_up_audio_write_fifo(pMicrophone->pHandle,
                                                pMicrophone->pNextSample,
                                                wordsToRead,
                                                ALT_UP_AUDIO_RIGHT);

            wordsRead = alt_up_audio_write_fifo(pMicrophone->pHandle,
                                                pMicrophone->pNextSample,
                                                wordsToRead,
                                                ALT_UP_AUDIO_LEFT);

            // Progress the read pointer forward
            pMicrophone->pNextSample += wordsRead;
        }
        else
        {
            alt_up_audio_disable_write_interrupt(pMicrophone->pHandle);
        }
    }
} // codecFifoISR

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

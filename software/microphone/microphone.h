/** @file   microphone.h
 *  @brief  Declarations, Structure, and Enumeration definitions for
 *          push-to-talk microphone driver.
 *
 *  Functions in the public API can be found under the *Functions* header
 *  below. All Enumerations are used for the internals of the scanner, all
 *  structures defined below are to be used with the public API.
 *
 *  @author Kyle O'Shaughnessy (koshaugh)
 */

#ifndef __MICROPHONE_H
#define __MICROPHONE_H

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#include <stdbool.h>
#include "includes.h"
#include "altera_up_avalon_audio.h"

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

#define MAX_RECORD_TIME_SECONDS     (10)
#define RECORDING_FREQUENCY_HERTZ   (32000)
#define RECORDING_BUFFER_SIZE       (RECORDING_FREQUENCY_HERTZ * MAX_RECORD_TIME_SECONDS)

/*****************************************************************************/
/* Structures                                                                */
/*****************************************************************************/

struct _Microphone;

typedef struct _Microphone
{
    alt_up_audio_dev   *pHandle;
    OS_EVENT           *pPushToTalkSemaphore;
    unsigned int        switchBaseAddress;
    unsigned int        recordingBuffer[RECORDING_BUFFER_SIZE];
    unsigned int       *pNextSample;
} Microphone;

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

// TODO: Add a playback function, just for testing
// TODO: Add an export function, takes in a byte buffer and fills it with 16-bit samples
Microphone* microphoneCreate(const char   *pName,
                             unsigned int  audioCoreIRQ,
                             unsigned int  switchBaseAddress,
                             unsigned int  switchIRQ);
void        microphoneDestroy(Microphone *pMicrophone);
void        microphoneWaitAndBeginRecording(Microphone *pMicrophone);
void        microphoneFinishRecording(Microphone *pMicrophone);

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

#endif // __MICROPHONE_H

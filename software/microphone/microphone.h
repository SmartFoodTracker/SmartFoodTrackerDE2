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
#ifdef TODO_EXPORTED_BUFFER
// TODO: add a typedef for an exported buffer
struct _Linear16Recoring;

typedef struct _Linear16Recording
{
    unsigned char   pRecording[RECORDING_BUFFER_SIZE * 2];
    size_t          size;
} Linear16Recording;
#endif // TODO_EXPORTED_BUFFER

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

Microphone* microphoneCreate(const char   *pName,
                             unsigned int  audioCoreIRQ,
                             unsigned int  switchBaseAddress,
                             unsigned int  switchIRQ);
void        microphoneDestroy(Microphone *pMicrophone);
void        microphoneWaitAndBeginRecording(Microphone *pMicrophone);
void        microphoneFinishRecording(Microphone *pMicrophone);
#ifdef TODO_EXTRA_FUNCTIONS
void        microphoneEnablePushToTalk(Microphone *pMicrophone);
void        microphoneDisablePushToTalk(Microphone *pMicrophone);
// TODO: Add an export function, takes in a byte buffer and fills it with 16-bit samples
void        microphoneExportLinear16(Microphone        *pMicrophone,
                                     Linear16Recording *pLinear16Recording);
#ifdef MICROPHONE_TESTING
// TODO: Add a playback function, just for testing
void        microphonePlayback(Microphone *pMicrophone);
#endif // MICROPHONE_TESTING
#endif // TODO_EXTRA_FUNCTIONS

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

#endif // __MICROPHONE_H

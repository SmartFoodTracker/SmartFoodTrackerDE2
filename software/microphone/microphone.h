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

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

#define MAX_RECORD_TIME_SECONDS 10

/*****************************************************************************/
/* Enumerations                                                              */
/*****************************************************************************/

/*****************************************************************************/
/* Structures                                                                */
/*****************************************************************************/

struct _Microphone;

typedef struct _Microphone
{
    unsigned int    switchBaseAddress;
    OS_EVENT       *pPushToTalkSemaphore;
} Microphone;

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

Microphone* microphoneCreate(unsigned int switchBaseAddress,
                             unsigned int switchIRQ);
void        microphoneDestroy(Microphone *pMicrophone);
void        microphoneWaitAndBeginRecording(Microphone *pMicrophone);
void        microphoneFinishRecording(Microphone *pMicrophone);

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

#endif // __MICROPHONE_H

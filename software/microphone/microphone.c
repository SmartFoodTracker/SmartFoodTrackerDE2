/** @file   micrphone.c
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
static void         releaseButtons(Microphone *pMicrophone);
static void         switchISR(void *pContext, alt_u32 id);

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

Microphone*
microphoneCreate()
{
    return NULL;
} // microphoneCreate

/*****************************************************************************/

void
microphoneDestroy(Microphone *pMicrophone)
{

} // microphoneDestroy

/*****************************************************************************/
/* Static Functions                                                          */
/*****************************************************************************/

static Microphone*
acquireMicrophone()
{
    return NULL;
} // acquireMicrophone

/*****************************************************************************/

static void
releaseButtons(Microphone *pMicrophone)
{

} // releaseButtons

/*****************************************************************************/

static void
switchISR(void *pContext, alt_u32 id)
{

} // switchISR

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

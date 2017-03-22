/** @file   input_tasks.h
 *  @brief  Public facing routines for facilitating food inventory tracker
 *          data entry
 *
 *  @author Andrew Bradshaw (abradsha), Kyle O'Shaughnessy (koshaugh)
 */

#ifndef __INPUT_TASKS_H
#define __INPUT_TASKS_H

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

#define FIT_MSG_READY           "FIT Ready"
#define FIT_MSG_SETUP_FAILED    "FIT Setup Failed"

/*****************************************************************************/
/* Enumerations                                                              */
/*****************************************************************************/

typedef enum _FITStatus
{
    FITStatusReady,
    FITStatusSetupFailed
} FITStatus;

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

void displayStatus(FITStatus status);
void FITSetup();

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

#endif // __INPUT_TASKS_H

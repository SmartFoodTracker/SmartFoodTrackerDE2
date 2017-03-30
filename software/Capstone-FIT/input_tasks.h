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
#define FIT_MSG_ITEM_ADDED      "Item added"
#define FIT_MSG_ITEM_REMOVED    "Item removed"
#define FIT_MSG_ITEM_UNKNOWN    "Unrecognized"


/*****************************************************************************/
/* Enumerations                                                              */
/*****************************************************************************/

typedef enum _FITStatus
{
    FITStatusReady,
    FITStatusSetupFailed,
    FITStatusItemAdded,
    FITStatusItemRemoved
} FITStatus;

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

void MicrophoneTask(void* pData);
void BarcodeTask(void* pData);
void ConfirmItem(char* pItemName, Buttons *pButtons);
void dispalyStatus(FITStatus status);
void displayStatusEx(FITStatus status, char *pOptionalString);
void FITSetup();

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

#endif // __INPUT_TASKS_H

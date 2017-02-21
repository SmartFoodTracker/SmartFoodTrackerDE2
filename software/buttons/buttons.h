/** @file   buttons.h
 *  @brief  Declarations, Structure, and Enumeration definitions for buttons
 *          driver.
 *
 *  Fucntions in the public API can be found under the *Functions* header
 *  below. All Enumerations are used for the internals of the scanner, all
 *  structures defined below are to be used with the public API.
 *
 *  @author Kyle O'Shaughnessy (koshaugh)
 */

#ifndef __BUTTONS_H
#define __BUTTONS_H

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#include <stdbool.h>
#include "includes.h"

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

#define BUTTONS_MESSAGE_QUEUE_SIZE  64

/*****************************************************************************/
/* Enumerations                                                              */
/*****************************************************************************/

typedef enum _Button
{
    ButtonAdd    = 0,
    ButtonRemove = 1,
    ButtonCancel = 2,
    ButtonMax // Index bound, add additional button IDs above this
} Button;

/*****************************************************************************/
/* Structures                                                                */
/*****************************************************************************/

struct _Buttons;
struct _ButtonContext;

typedef struct _Buttons
{
    struct _ButtonContext  *isrContexts[ButtonMax];
    OS_EVENT           	   *pButtonPressQueue;
    void               	   *pButtonPressQueueData[BUTTONS_MESSAGE_QUEUE_SIZE];
} Buttons;

typedef struct _ButtonContext
{
    struct _Buttons    	   *pButtons;
    unsigned int        	irq;
    unsigned int        	baseAddress;
    Button              	buttonID;
} ButtonContext;

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

Buttons*    buttonsCreate();
void        buttonsDestroy(Buttons *pButtons);
INT8U       buttonsInitButton(Buttons      *pButtons,
                              Button        buttonID,
                              unsigned int  baseAddress,
                              unsigned int  irq);
void        buttonsEnableButton(Buttons *pButtons, Button buttonID);
void        buttonsDisableButton(Buttons *pButtons, Button buttonID);
void        buttonsEnableAll(Buttons *pButtons);
void        buttonsDisableAll(Buttons *pButtons);
Button      buttonsGetButtonPress(Buttons *pButtons);

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

#endif // __BUTTONS_H

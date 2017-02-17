/** @file   barcode_scanner.h
 *  @brief  Declarations, Structure, and Enumeration definitions for barcode
 *          scanner driver.
 *
 *  Fucntions in the public API can be found under the *Functions* header
 *  below. All Enumerations are used for the internals of the scanner, all
 *  structures defined below are to be used with the public API.
 *
 *  @author Kyle O'Shaughnessy (koshaugh)
 */

#ifndef __BARCODE_SCANNER_H
#define __BARCODE_SCANNER_H

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#include <stdbool.h>
#include "includes.h"
#include "altera_up_avalon_ps2.h"
#include "altera_up_ps2_keyboard.h"

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

#define BARCODE_MESSAGE_QUEUE_SIZE  64
#define MAX_BARCODE_LENGTH          48
#define MAX_KEY_PRESS_LENGTH        16

#define BARCODE_CONTROL             "L CTRL"
#define BARCODE_SHIFT               "L SHFT"
#define BARCODE_DELIMITER           "ENTER"

/*****************************************************************************/
/* Enumerations                                                              */
/*****************************************************************************/

typedef enum _DecodeStatus
{
    DecodeStatusComplete,
    DecodeStatusNotComplete
} DecodeStatus;

typedef enum _KeyPosition
{
    KeyPositionUp,
    KeyPositionDown
} KeyPosition;

/*****************************************************************************/
/* Structures                                                                */
/*****************************************************************************/

typedef struct _Barcode
{
    char            pString[MAX_BARCODE_LENGTH];
} Barcode;

typedef struct _EncodedKeyPress
{
    KB_CODE_TYPE    decodeMode;
    INT8U           encodedValue;
} EncodedKeyPress;

typedef struct _BarcodeScanner
{
    alt_up_ps2_dev *pHandle;
    OS_EVENT       *pBarcodeKeyPressQueue;
    void           *pBarcodeKeyPressQueueData[BARCODE_MESSAGE_QUEUE_SIZE];
    KeyPosition     keyPosition;
    bool            enabled;
} BarcodeScanner;

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

BarcodeScanner* barcodeScannerCreate(const char   *pName,
                                     unsigned int  baseAddress,
                                     unsigned int  irq);
void            barcodeScannerDestroy(BarcodeScanner *pBarcodeScanner);
void            barcodeScannerDecode(BarcodeScanner *pBarcodeScanner,
                                     Barcode        *pBarcode);

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

#endif /* __BARCODE_SCANNER_H */

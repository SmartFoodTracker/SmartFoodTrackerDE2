#ifndef __BARCODE_SCANNER_H
#define __BARCODE_SCANNER_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "includes.h"
#include "io.h"
#include "altera_up_avalon_ps2.h"
#include "altera_up_ps2_keyboard.h"


/* Constants */
#define BARCODE_MESSAGE_QUEUE_SIZE  64
#define MAX_BARCODE_LENGTH          48
#define MAX_KEY_PRESS_LENGTH        16
/* Constants*/


/* Enumerations */
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
/* Enumerations */


/* Structures */
typedef struct _EncodedKeyPress
{
    KB_CODE_TYPE    decodeMode;
    INT8U           encodedValue;
} EncodedKeyPress;

typedef struct _Barcode
{
    char pString[MAX_BARCODE_LENGTH];
} Barcode;

typedef struct _BarcodeScanner
{
    alt_up_ps2_dev *pHandle;
    OS_EVENT       *pBarcodeKeyPressQueue;
    void           *pBarcodeKeyPressQueueData[BARCODE_MESSAGE_QUEUE_SIZE];
    KeyPosition     keyPosition;
    bool            enabled;
} BarcodeScanner;
/* Structures */


/* Functions */
BarcodeScanner* barcodeScannerCreate();
void            barcodeScannerDestroy(BarcodeScanner *pBarcodeScanner);
void            barcodeScannerDecode(BarcodeScanner *pBarcodeScanner, Barcode *pBarcode);
/* Functions */


#endif /* __BARCODE_SCANNER_H */

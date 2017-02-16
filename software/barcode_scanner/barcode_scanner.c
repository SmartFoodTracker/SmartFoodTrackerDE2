/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "io.h"
#include "barcode_scanner.h"

/*****************************************************************************/
/* Declarations                                                              */
/*****************************************************************************/

static BarcodeScanner*  acquireBarcodeScanner();
static void             releaseBarcodeScanner(BarcodeScanner *pBarcodeScanner);
static INT8U            initHandle(BarcodeScanner *pBarcodeScanner,
                                   const char     *pName,
                                   unsigned int    baseAddress,
                                   unsigned int    irq);
static INT8U            initQueue(BarcodeScanner *pBarcodeScanner);
static void             dataLineISR(void *pContext, alt_u32 id);

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

BarcodeScanner*
barcodeScannerCreate(const char   *pName,
                     unsigned int  baseAddress,
                     unsigned int  irq)
{
    INT8U           status          = OS_NO_ERR;
    BarcodeScanner *pBarcodeScanner = acquireBarcodeScanner();

    if (pBarcodeScanner == NULL)
    {
        status = OS_ERR_PDATA_NULL;
    }

    // Register PS2 device handle with interrupt
    if (status == OS_NO_ERR)
    {
        status = initHandle(pBarcodeScanner, pName, baseAddress, irq);
    }

    // Initialize message queue
    if (status == OS_NO_ERR)
    {
        status = initQueue(pBarcodeScanner);
    }

    // Cleanup if an error has occured
    if (status != OS_NO_ERR)
    {
        releaseBarcodeScanner(pBarcodeScanner);
        pBarcodeScanner = NULL;
    }

    return pBarcodeScanner;
} // barcodeScannerCreate

/*****************************************************************************/

void
barcodeScannerDestroy(BarcodeScanner *pBarcodeScanner)
{
    releaseBarcodeScanner(pBarcodeScanner);
} // barcodeScannerDestroy

/*****************************************************************************/

void
barcodeScannerDecode(BarcodeScanner *pBarcodeScanner, Barcode *pBarcode)
{
    char                pKeyPressString[MAX_KEY_PRESS_LENGTH];
    EncodedKeyPress    *pEncodedKeyPress    = NULL;
    INT8U               queueError          = OS_NO_ERR;
    DecodeStatus        status              = DecodeStatusNotComplete;

    if (pBarcodeScanner && pBarcode)
    {
        // Clear barcode
        memset(pBarcode, 0, sizeof(Barcode));

        // Begin decode process
        while (status == DecodeStatusNotComplete)
        {
            // Receive encoded key press from ISR
            pEncodedKeyPress = (EncodedKeyPress *)OSQPend(pBarcodeScanner->pBarcodeKeyPressQueue,
                                                          0,
                                                          &queueError);

            if ((queueError == OS_NO_ERR) && (pEncodedKeyPress != NULL))
            {
                // Decode the key press
                translate_make_code(pEncodedKeyPress->decodeMode,
                                    pEncodedKeyPress->encodedValue,
                                    pKeyPressString);

                // Toggle decoding around "L CTRL"
                if (strcmp(pKeyPressString, "L CTRL") == 0)
                {
                    pBarcodeScanner->enabled = !(pBarcodeScanner->enabled);
                }

                // Begin filtering out repeated keys, delimiters, etc.
                if (pBarcodeScanner->enabled &&
                    (strcmp(pKeyPressString, "L SHFT") != 0) &&
                    (strcmp(pKeyPressString, "L CTRL") != 0))
                {
                    // Update the key position from the last decode sequence
                    if (pBarcodeScanner->keyPosition == KeyPositionUp)
                    {
                        pBarcodeScanner->keyPosition = KeyPositionDown;
                        if (strcmp(pKeyPressString, "ENTER") == 0)
                        {
                            status = DecodeStatusComplete;
                        }
                        else
                        {
                            strncat(pBarcode->pString,
                                    pKeyPressString,
                                    strlen(pKeyPressString));
                        }
                    }
                    else
                    {
                        pBarcodeScanner->keyPosition = KeyPositionUp;
                    }
                }

                // Free encoded key press passed from ISR
                free(pEncodedKeyPress);
                pEncodedKeyPress = NULL;
            }
        }
    }
} // barcodeScannerDecode

/*****************************************************************************/
/* Static Functions                                                          */
/*****************************************************************************/

static BarcodeScanner*
acquireBarcodeScanner()
{
    BarcodeScanner *pBarcodeScanner = (BarcodeScanner *) malloc(sizeof(BarcodeScanner));
    if (pBarcodeScanner)
    {
        pBarcodeScanner->pHandle                = NULL;
        pBarcodeScanner->pBarcodeKeyPressQueue  = NULL;
        pBarcodeScanner->enabled                = true;
        pBarcodeScanner->keyPosition            = KeyPositionUp;
    }

    return pBarcodeScanner;
} // acquireBarcodeScanner

/*****************************************************************************/

static void
releaseBarcodeScanner(BarcodeScanner *pBarcodeScanner)
{
    if (pBarcodeScanner)
    {
        free(pBarcodeScanner);
    }
} // releaseBarcodeScanner

/*****************************************************************************/

static INT8U
initHandle(BarcodeScanner *pBarcodeScanner,
           const char     *pName,
           unsigned int    baseAddress,
           unsigned int    irq)
{
    INT8U status = OS_NO_ERR;

    if (pBarcodeScanner && ((pBarcodeScanner->pHandle = alt_up_ps2_open_dev(pName)) != NULL))
    {
        pBarcodeScanner->pHandle->base          = baseAddress;
        pBarcodeScanner->pHandle->irq_id        = irq;
        pBarcodeScanner->pHandle->timeout       = 0;
        pBarcodeScanner->pHandle->device_type   = PS2_KEYBOARD;

        set_keyboard_rate(pBarcodeScanner->pHandle, 0);

        alt_up_ps2_enable_read_interrupt(pBarcodeScanner->pHandle);

        status = alt_irq_register(pBarcodeScanner->pHandle->irq_id,
                                  (void *)pBarcodeScanner,
                                  dataLineISR);
    }
    else
    {
        status = OS_ERR_PDATA_NULL;
    }

    return status;
} // initHandle

/*****************************************************************************/

static INT8U
initQueue(BarcodeScanner *pBarcodeScanner)
{
    INT8U status = OS_NO_ERR;
    pBarcodeScanner->pBarcodeKeyPressQueue = OSQCreate(pBarcodeScanner->pBarcodeKeyPressQueueData,
                                                       BARCODE_MESSAGE_QUEUE_SIZE);
    if (pBarcodeScanner->pBarcodeKeyPressQueue == NULL)
    {
        status = OS_ERR_PDATA_NULL;
    }

    return status;
} // initQueue

/*****************************************************************************/

static void
dataLineISR(void *pContext, alt_u32 id)
{
    BarcodeScanner     *pBarcodeScanner = (BarcodeScanner *)pContext;
    EncodedKeyPress    *pData           = NULL;
    KB_CODE_TYPE        decodeMode      = KB_INVALID_CODE;
    alt_u8              encodedValue    = 0;
    char                asciiValue      = '\0';

    // Read byte from device and clear interrupt
    if (decode_scancode(pBarcodeScanner->pHandle, &decodeMode, &encodedValue, &asciiValue) == 0)
    {
        pData = (EncodedKeyPress *)malloc(sizeof(EncodedKeyPress));
        if (pData != NULL)
        {
            pData->decodeMode   = decodeMode;
            pData->encodedValue = encodedValue;
            OSQPost(pBarcodeScanner->pBarcodeKeyPressQueue, pData);
        }
    }
} // dataLineISR

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

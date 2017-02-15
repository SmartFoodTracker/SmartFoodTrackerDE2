#include "barcode_scanner.h"

static void barcodeScannerISR(void *pContext, alt_u32 id);

BarcodeScanner*
barcodeScannerCreate()
{
    INT8U           status          = OS_NO_ERR;
    BarcodeScanner *pBarcodeScanner = (BarcodeScanner *) malloc(sizeof(BarcodeScanner));

    if (pBarcodeScanner == NULL)
    {
        status = OS_ERR_PDATA_NULL;
    }
    else
    {
        // Set defaults
        pBarcodeScanner->pHandle                = NULL;
        pBarcodeScanner->pBarcodeKeyPressQueue  = NULL;
        pBarcodeScanner->enabled                = true;
        pBarcodeScanner->keyPosition            = KeyPositionUp;

        // Setup device handle
        if ((pBarcodeScanner->pHandle = alt_up_ps2_open_dev(BARCODE_SCANNER_PS2_NAME)) != NULL)
        {
            pBarcodeScanner->pHandle->base          = BARCODE_SCANNER_PS2_BASE;
            pBarcodeScanner->pHandle->irq_id        = BARCODE_SCANNER_PS2_IRQ;
            pBarcodeScanner->pHandle->timeout       = 0;
            pBarcodeScanner->pHandle->device_type   = PS2_KEYBOARD;

            set_keyboard_rate(pBarcodeScanner->pHandle, 0);

            alt_up_ps2_enable_read_interrupt(pBarcodeScanner->pHandle);

            status = alt_irq_register(pBarcodeScanner->pHandle->irq_id,
                                      (void *)pBarcodeScanner,
                                      barcodeScannerISR);
        }
        else
        {
            status = OS_ERR_PDATA_NULL;
        }

        // Initialize synchronized data structures
        pBarcodeScanner->pBarcodeKeyPressQueue = OSQCreate(pBarcodeScanner->pBarcodeKeyPressQueueData,
                                                           BARCODE_MESSAGE_QUEUE_SIZE);
        if (pBarcodeScanner->pBarcodeKeyPressQueue == NULL)
        {
            status = OS_ERR_PDATA_NULL;
        }

        if (status != OS_NO_ERR)
        {
            free(pBarcodeScanner);
            pBarcodeScanner = NULL;
        }
    }

    return pBarcodeScanner;
}

void
barcodeScannerDestroy(BarcodeScanner *pBarcodeScanner)
{
    free(pBarcodeScanner);
}

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
            else
            {
                printf("Error pending on pBarcodeKeyPressQueue: %d\n", queueError);
            }
        }
    }
}

static void
barcodeScannerISR(void *pContext, alt_u32 id)
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
}

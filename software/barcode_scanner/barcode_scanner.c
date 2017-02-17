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

/**
 * @brief               Allocates and intializes a BarcodeScanner object.
 * @details             Allocates BarcodeScanner object on the heap, 
 *                      intiailizes the device handle, registers the ISR, and
 *                      initializes the key press queue.
 * 
 * @param pName         Name of the dev port associated with the PS2 port.
 * @param baseAddress   Base address of the memory mapped device.
 * @param irq           IRQ number of the memory mapped device.
 * @return              A new BarcodeScanner object. 
 */
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

/**
 * @brief                   Destroys BarcodeScanner object.
 * @details                 Should be called if barcodeScannerCreate was used
 *                          to create the object. This will free all resources
 *                          associated with the object and the object itself.
 * 
 * @param pBarcodeScanner   Pointer to BarcodeScanner to be destroyed.
 */
void
barcodeScannerDestroy(BarcodeScanner *pBarcodeScanner)
{
    releaseBarcodeScanner(pBarcodeScanner);
} // barcodeScannerDestroy

/*****************************************************************************/

/**
 * @brief                   Constructs a barcode from individual key presses.
 * @details                 This is a blocking function. Key presses sent from
 *                          dataLineISR are consumed in this routine, this
 *                          queue pend has no timeout. Duplicate keypresses,
 *                          SHIFTs, Control characters, and ENTERs are ignored.
 *                          This routine will return when the decoding of a
 *                          complete barcode finishes.
 * 
 * @param pBarcodeScanner   Pointer to barcode scanner (in/out)
 * @param pBarcode          Pointer to barcode to be filled in (out)
 */
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

/**
 * @brief   Allocate a BarcodeScanner object and intialize it
 * @details Allocates a BarcodeScanner on the heap and sets all members to
 *          defaults
 * 
 * @return  Pointer to a BarcodeScanner object or NULL in the case of failed
 *          alloc.
 */
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

/**
 * @brief                   Release resources associated with a BarcodeScanner.
 * @details                 This should be called for any device created with 
 *                          acquireBarcodeScanner().
 * 
 * @param pBarcodeScanner   Pointer to BarcodeScanner object to be released.
 */
static void
releaseBarcodeScanner(BarcodeScanner *pBarcodeScanner)
{
    if (pBarcodeScanner)
    {
        free(pBarcodeScanner);
    }
} // releaseBarcodeScanner

/*****************************************************************************/

/**
 * @brief                   Initialize PS2 device handle and register ISR.
 * @details                 Open dev handle for ps2 port; assign base address
 *                          and irq; register isr.
 * 
 * @param pBarcodeScanner   Parent object, owner of device handle
 * @param pName             Name of dev port for the ps2 device.
 * @param baseAddress       Base address of the memory mapped device.
 * @param irq               IRQ number for memory mapped device.
 */
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

/**
 * @brief                   Initialize the barcode keypress queue.
 * @details                 Simply associated the queue data with a new queue
 *                          which is assigned to the barcode scanner.
 * 
 * @param pBarcodeScanner   Pointer to parent object.
 */
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

/**
 * @brief           Interrupt service routine for PS2 data line.
 * @details         Decode and push new keypresses onto a synchronized queue.
 *                  The data pushed onto the queue should be freed after being
 *                  consumed.
 * 
 * @param pContext  Pointer to a BarcodeScanner object, this is passed in 
 *                  during initHandle.
 * @param id        Interrupt id, currently unused.
 */
static void
dataLineISR(void *pContext, alt_u32 id)
{
    BarcodeScanner     *pBarcodeScanner = (BarcodeScanner *) pContext;
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

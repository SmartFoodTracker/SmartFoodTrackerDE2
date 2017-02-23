/** @file   barcode_scanner.c
 *  @brief  Source for public and private routines used for barcode scanning
 *
 *  Driver for the PS2 barcode scanner. All non-static functions are
 *  effectively part of the public API, those that are static should be
 *  considered private.
 *
 *  @author Kyle O'Shaughnessy (koshaugh)
 */

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
static EncodedKeyPress* getNextKeyPress(BarcodeScanner *pBarcodeScanner);
static void             toggleKeyPosition(KeyPosition *pKeyPosition);
static bool             isControlKey(const char *pKeyPressString);
static bool             isValidKey(const char *pKeyPressString);
static bool             isDelimiterKey(const char *pKeyPressString);

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

/**
 * @brief      Allocates and intializes a BarcodeScanner object on the heap.
 *             intiailizes the device handle, registers the ISR, and
 *             initializes the key press queue. barcodeScannerDestroy(...)
 *             should be called to cleanup this object.
 *
 * @param[in]  pName        Name of the dev port associated with the PS2 port
 * @param[in]  baseAddress  Base address of the memory mapped device
 * @param[in]  irq          IRQ number of the memory mapped device
 *
 * @return     A new BarcodeScanner object
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
 * @brief      Destroys BarcodeScanner object. Should be called if
 *             barcodeScannerCreate was used to create the object. This will
 *             free all resources associated with the object and the object
 *             itself.
 *
 * @param[in]  pBarcodeScanner  Pointer to BarcodeScanner to be destroyed
 */
void
barcodeScannerDestroy(BarcodeScanner *pBarcodeScanner)
{
    releaseBarcodeScanner(pBarcodeScanner);
} // barcodeScannerDestroy

/*****************************************************************************/

/**
 * @brief      Constructs a barcode from individual key presses. This is a
 *             blocking function. Key presses sent from dataLineISR are
 *             consumed in this routine, this queue pend has no timeout.
 *             Duplicate keypresses, SHIFTs, wrapped Control characters, and
 *             ENTERs are ignored. This routine will return when the decoding
 *             of a complete barcode finishes.
 *
 * @param[in]  pBarcodeScanner  Pointer to barcode scanner
 * @param[out] pBarcode         Pointer to barcode to be filled in
 */
void
barcodeScannerDecode(BarcodeScanner *pBarcodeScanner, Barcode *pBarcode)
{
    char                pKeyPressString[MAX_KEY_PRESS_LENGTH];
    EncodedKeyPress    *pEncodedKeyPress    = NULL;
    DecodeStatus        status              = DecodeStatusNotComplete;

    if (pBarcodeScanner && pBarcode)
    {
        // Clear barcode
        memset(pBarcode, 0, sizeof(Barcode));

        // Begin decode process
        while (status == DecodeStatusNotComplete)
        {
            // Fetch next encoded key press
            pEncodedKeyPress = getNextKeyPress(pBarcodeScanner);

            if (pEncodedKeyPress != NULL)
            {
                // Decode the key press
                translate_make_code(pEncodedKeyPress->decodeMode,
                                    pEncodedKeyPress->encodedValue,
                                    pKeyPressString);

                // Toggle decoding around CTRL, these come in pairs (up/down)
                // This prevents carriage returns from being processed
                if (isControlKey(pKeyPressString))
                {
                    pBarcodeScanner->enabled = !(pBarcodeScanner->enabled);
                }

                // Begin filtering out repeated keys, delimiters, etc.
                if (pBarcodeScanner->enabled && isValidKey(pKeyPressString))
                {
                    // Toggle the key position of the virtual keyboard
                    toggleKeyPosition(&(pBarcodeScanner->keyPosition));

                    // Only process "down" key presses
                    if (pBarcodeScanner->keyPosition == KeyPositionDown)
                    {
                        if (isDelimiterKey(pKeyPressString))
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
                }

                // Free encoded key press passed from ISR
                free(pEncodedKeyPress);
                pEncodedKeyPress = NULL;
            }
        }
    }
} // barcodeScannerDecode

/*****************************************************************************/

/**
 * @brief      { function_description }
 *
 * @param[in]  pBarcodeScanner  The barcode scanner
 */
void
barcodeScannerEnable(BarcodeScanner *pBarcodeScanner)
{
	if (pBarcodeScanner)
	{
		alt_up_ps2_enable_read_interrupt(pBarcodeScanner->pHandle);
	}
} // barcodeScannerEnable

/*****************************************************************************/

/**
 * @brief      { function_description }
 *
 * @param[in]  pBarcodeScanner  The barcode scanner
 */
void
barcodeScannerDisable(BarcodeScanner *pBarcodeScanner)
{
	if (pBarcodeScanner)
	{
		alt_up_ps2_disable_read_interrupt(pBarcodeScanner->pHandle);
	}
} // barcodeScannerDisable

/*****************************************************************************/
/* Static Functions                                                          */
/*****************************************************************************/

/**
 * @brief      Allocate a BarcodeScanner object on heap and intialize it. All
 *             members set to defaults. Should be paired with an eventual call
 *             to releaseBarcodeScanner
 *
 * @return     Pointer to a BarcodeScanner object or NULL in the case of failed
 *             malloc
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
 * @brief      Release resources associated with a BarcodeScanner. This should
 *             be called for any device created with acquireBarcodeScanner().
 *
 * @param[in]  pBarcodeScanner  Pointer to BarcodeScanner object to be released
 */
static void
releaseBarcodeScanner(BarcodeScanner *pBarcodeScanner)
{
    INT8U queueError = OS_NO_ERR;
    if (pBarcodeScanner)
    {
        alt_up_ps2_disable_read_interrupt(pBarcodeScanner->pHandle);

        if(pBarcodeScanner->pBarcodeKeyPressQueue)
        {
            OSQDel(pBarcodeScanner->pBarcodeKeyPressQueue,
                   OS_DEL_ALWAYS,
                   &queueError);

            pBarcodeScanner->pBarcodeKeyPressQueue = NULL;
        }

        free(pBarcodeScanner);
    }
} // releaseBarcodeScanner

/*****************************************************************************/

/**
 * @brief      Initialize PS2 device handle and register ISR. Opens dev handle
 *             for ps2 port; assigns base address and irq; registers isr.
 *
 * @param[in]  pBarcodeScanner  Parent object, owner of device handle
 * @param[in]  pName            Name of dev port for the ps2 device
 * @param[in]  baseAddress      Base address of the memory mapped device
 * @param[in]  irq              IRQ number for memory mapped device
 *
 * @return     OS_NO_ERR if no errors, OS_ERR_PDATA_NULL if handle fails to
 *             initialize.
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
        pBarcodeScanner->pHandle->base        = baseAddress;
        pBarcodeScanner->pHandle->irq_id      = irq;
        pBarcodeScanner->pHandle->timeout     = 0;
        pBarcodeScanner->pHandle->device_type = PS2_KEYBOARD;

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
 * @brief      Initialize the barcode keypress queue. Simply associates the
 *             queue data with a new queue which is assigned to the barcode
 *             scanner.
 *
 * @param[in]  pBarcodeScanner  Pointer to parent object
 *
 * @return     OS_NO_ERR if no error, OS_ERR_PDATA_NULL if created queue is
 *             NULL
 */
static INT8U
initQueue(BarcodeScanner *pBarcodeScanner)
{
    INT8U status = OS_NO_ERR;

    if (pBarcodeScanner)
    {
        pBarcodeScanner->pBarcodeKeyPressQueue = OSQCreate(pBarcodeScanner->pBarcodeKeyPressQueueData,
                                                           BARCODE_MESSAGE_QUEUE_SIZE);
        if (pBarcodeScanner->pBarcodeKeyPressQueue == NULL)
        {
            status = OS_ERR_PDATA_NULL;
        }
    }
    else
    {
        status = OS_ERR_PDATA_NULL;
    }

    return status;
} // initQueue

/*****************************************************************************/

/**
 * @brief      Interrupt service routine for PS2 data line. Decodes and pushes
 *             new keypresses onto a synchronized queue. The data pushed onto
 *             the queue should be freed after being consumed.
 *
 * @param[in]  pContext  Pointer to a BarcodeScanner object, this is passed in
 *                       during initHandle.
 * @param[in]  id        UNUSED_PARAMETER
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
    if (decode_scancode(pBarcodeScanner->pHandle, &decodeMode,
                        &encodedValue, &asciiValue) == 0)
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

/**
 * @brief      Fetches next available key press from the scanner's processing
 *             queue. Pends on pBarcodeKeyPressQueue indefinitely to get next
 *             keypress.
 *
 * @param[in]  pBarcodeScanner  Pointer to a BarcodeScanner object with a
 *                              keypress available
 *
 * @return     Pointer to an EncodedKeyPress object, this needs to be freed by
 *             the consumer after use
 */
static EncodedKeyPress*
getNextKeyPress(BarcodeScanner *pBarcodeScanner)
{
    EncodedKeyPress *pEncodedKeyPress = NULL;
    INT8U            queueError       = OS_NO_ERR;

    if (pBarcodeScanner)
    {
        pEncodedKeyPress = (EncodedKeyPress *)OSQPend(pBarcodeScanner->pBarcodeKeyPressQueue,
                                                      0,
                                                      &queueError);
        if (queueError != OS_NO_ERR)
        {
            pEncodedKeyPress = NULL;
        }
    }

    return pEncodedKeyPress;
} // getNextKeyPress

/*****************************************************************************/

/**
 * @brief      { function_description }
 *
 * @param      pKeyPosition  The key position
 */
static void
toggleKeyPosition(KeyPosition *pKeyPosition)
{
    if (pKeyPosition)
    {
        if (*pKeyPosition == KeyPositionUp)
        {
            *pKeyPosition = KeyPositionDown;
        }
        else
        {
            *pKeyPosition = KeyPositionUp;
        }
    }
} // toggleKeyPosition

/*****************************************************************************/

/**
 * @brief      Determines if control key.
 *
 * @param[in]  pKeyPressString  The key press string
 *
 * @return     True if control key, False otherwise.
 */
static bool
isControlKey(const char *pKeyPressString)
{
    bool bReturn = false;

    if (pKeyPressString)
    {
        bReturn = (strcmp(pKeyPressString, BARCODE_CONTROL) == 0);
    }

    return bReturn;
} // isControlKey

/*****************************************************************************/

/**
 * @brief      Determines if valid key.
 *
 * @param[in]  pKeyPressString  The key press string
 *
 * @return     True if valid key, False otherwise.
 */
static bool
isValidKey(const char *pKeyPressString)
{
    bool bReturn = false;

    if (pKeyPressString)
    {
        bReturn = ((strcmp(pKeyPressString, BARCODE_SHIFT)   != 0) &&
                   (strcmp(pKeyPressString, BARCODE_CONTROL) != 0));
    }

    return bReturn;
} // isValidKey

/*****************************************************************************/

/**
 * @brief      Determines if delimiter key.
 *
 * @param[in]  pKeyPressString  The key press string
 *
 * @return     True if delimiter key, False otherwise.
 */
static bool
isDelimiterKey(const char *pKeyPressString)
{
    bool bReturn = false;

    if (pKeyPressString)
    {
        bReturn = (strcmp(pKeyPressString, BARCODE_DELIMITER) == 0);
    }

    return bReturn;
} // isDelimiterKey

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

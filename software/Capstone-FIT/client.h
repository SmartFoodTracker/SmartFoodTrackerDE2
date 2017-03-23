/** @file   client.h
 *  @brief  Public facing routines to facilitate http requests to our server
 *
 *  @author Andrew Bradshaw (abradsha), Kyle O'Shaughnessy (koshaugh)
 */

#ifndef __CLIENT_H
#define __CLIENT_H

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

#define FIT_PORT            80
#define FIT_IP_ADDR         "13.56.5.40"
#define FIT_MAX_HTTP_SIZE   500000
#define FIT_MAX_BODY_SIZE   1000

/*****************************************************************************/
/* Structures                                                                */
/*****************************************************************************/

typedef struct _FITRequest
{
    char pRequest[FIT_MAX_HTTP_SIZE];
    char pResponse[FIT_MAX_HTTP_SIZE];
    char pBody[FIT_MAX_BODY_SIZE];
} FITRequest;
/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

int translate_barcode(char *pBarcodeString,
                      char *pItemString);
int translate_audio(char *pAudioRecording,
                    long  audioLengthBytes,
                    char *pItemString);
int add_item(char *pItemString);
int remove_item(char *pItemString);

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

#endif // __CLIENT_H

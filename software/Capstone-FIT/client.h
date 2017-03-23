/** @file   client.h
 *  @brief  Public facing routines to facilitate http requests to our server
 *
 *  @author Andrew Bradshaw (abradsha), Kyle O'Shaughnessy (koshaugh)
 */

#ifndef __CLIENT_H
#define __CLIENT_H

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

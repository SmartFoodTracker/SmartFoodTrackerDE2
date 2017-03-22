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

int translate_barcode(char* barcode, char* response);
int translate_audio(char* audio, long audio_length, char* response);
int add_item(char* item);
int remove_item(char *item);

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

#endif // __CLIENT_H

/*
 * client.h
 *
 *  Created on: Feb 10, 2017
 *      Author: abradsha
 */

#ifndef CLIENT_H_
#define CLIENT_H_

int translate_barcode(char* barcode, char* response);
int translate_audio(char* audio, long audio_length, char* response);
int add_item(char* item);
int remove_item(char *item);

#endif /* CLIENT_H_ */

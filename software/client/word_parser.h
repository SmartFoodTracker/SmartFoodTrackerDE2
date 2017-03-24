/*
 * client.h
 *
 *  Created on: March 24, 2017
 *      Author: abradsha
 */

#ifndef WORD_PARSER_H_
#define WORD_PARSER_H_

typedef enum _Command
{
    CommandAdd    = 0,
    CommandRemove = 1,
    CommandNothing
} Command;

Command parse_command(char* original_text, char* stripped_text);
int parse_number(char* original_text, char* stripped_text);

#endif /* WORD_PARSER_H_ */

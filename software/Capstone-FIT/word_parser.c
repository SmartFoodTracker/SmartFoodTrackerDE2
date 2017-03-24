/** @file   word_parser.c
 *  @brief  Routines to facilitate parsing text from strings
 *
 *  Used to detect key words in string and retrieve the resulsts
 *
 *  @author Andrew Bradshaw (abradsha)
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "word_parser.h"

/**
 * @brief      Get the position of the first space in a string
 *
 * @param[in]  string     String you want to find the first space for
 *
 * @return     position of space if successful, -1 otherwise
 */
int get_first_space(char* string) {
    int pos = 0;
    while (string[pos] != '\0') {
        if (string[pos] == ' ') {
            return pos;
        }
        pos += 1;
    }
    return -1;
}

/**
 * @brief      Get and extract the first command if there is one
 *
 * @param[in]     original_text     Original text that you wish to check if there is a command
 * @param[inout]  stripped_text     Text with command removed, buffer must be allocated before hand
 *
 *
 * @return     Command representing the first word in the text, or CommandNothing if there was no command
 */
Command parse_command(char* original_text, char* stripped_text) {
    int first_space = get_first_space(original_text);
    char first_word[50];
    Command c = CommandNothing;
    // There were no spaces to make a first word, return and copy original text
    if (first_space == -1) {
        strncpy(stripped_text, original_text, strlen(original_text) + 1);
        return c;
    }
    strncpy(first_word, original_text, first_space);
    first_word[first_space] = '\0';
    if (strcmp(first_word, "add") == 0) {
        c = CommandAdd;
    }
    else if (strcmp(first_word, "Add") == 0) {
        c = CommandAdd;
    }
    else if (strcmp(first_word, "Delete") == 0) {
        c = CommandRemove;
    }
    else if (strcmp(first_word, "delete") == 0) {
        c = CommandRemove;
    }
    else if (strcmp(first_word, "Remove") == 0) {
        c = CommandRemove;
    }
    else if (strcmp(first_word, "remove") == 0) {
        c = CommandRemove;
    }
    if (c == CommandRemove || c == CommandAdd) {
        // Copy rest of string into new string
        int new_length = strlen(original_text) - first_space + 1;
        strncpy(stripped_text, original_text + first_space + 1, new_length);
    } else {
        // Copy original string into new string
        strncpy(stripped_text, original_text, strlen(original_text) + 1);
    }
    return c;
}

/**
 * @brief      Convert the first word in a string into an int
 *
 * @param[in]  original_text   The text you want to check the first word for
 *
 * @return     int value of first word in string, if there is no key word returns 1
 */
int parse_number(char* original_text, char* stripped_text) {
    int first_space = get_first_space(original_text);
    char first_word[50];
    int amount = 0;
    // There were no spaces to make a first word, only one copy
    if (first_space == -1) {
	strncpy(stripped_text, original_text, strlen(original_text) + 1);
        return 1;
    }
    strncpy(first_word, original_text, first_space);
    first_word[first_space] = '\0';
    if (strcmp(first_word, "one") == 0 || strcmp(first_word, "1") == 0) {
        amount = 1;
    }
    else if (strcmp(first_word, "two") == 0 || strcmp(first_word, "2") == 0
             || strcmp(first_word, "too") == 0|| strcmp(first_word, "to") == 0) {
        amount = 2;
    }
    else if (strcmp(first_word, "three") == 0) {
        amount = 3;
    }
    else if (strcmp(first_word, "four") == 0 || strcmp(first_word, "4") == 0 || strcmp(first_word, "for") == 0) {
        amount = 4;
    }
    else if (strcmp(first_word, "five") == 0 || strcmp(first_word, "5") == 0) {
        amount = 5;
    }
    else if (strcmp(first_word, "six") == 0 || strcmp(first_word, "6") == 0) {
        amount = 6;
    }
    else if (strcmp(first_word, "seven") == 0 || strcmp(first_word, "7") == 0) {
        amount = 7;
    }
    else if (strcmp(first_word, "eight") == 0 || strcmp(first_word, "8") == 0) {
        amount = 8;
    }
    else if (strcmp(first_word, "nine") == 0 || strcmp(first_word, "9") == 0) {
        amount = 9;
    }
    else if (strcmp(first_word, "ten") == 0 || strcmp(first_word, "10") == 0) {
        amount = 10;
    } else {
        strncpy(stripped_text, original_text, strlen(original_text) + 1);
    }
    if (amount > 0) {
        // Copy rest of string into new string
        int new_length = strlen(original_text) - first_space + 1;
        strncpy(stripped_text, original_text + first_space + 1, new_length);
    } else {
        amount = 1;
    }
    return amount;
}

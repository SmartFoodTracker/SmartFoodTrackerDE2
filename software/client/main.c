#include "client.h"
#include "word_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

char *buffer;

// Taken from http://stackoverflow.com/questions/22059189/read-a-file-as-byte-array
long readFile() {
    FILE *fileptr;
    long filelen;

    fileptr = fopen("bridge.raw", "rb");  // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
    filelen = ftell(fileptr);             // Get the current byte offset in the file
    rewind(fileptr);                      // Jump back to the beginning of the file

    buffer = (char *)malloc((filelen)*sizeof(char)); // Enough memory for file
    fread(buffer, filelen, 1, fileptr); // Read in the entire file
    fclose(fileptr); // Close the file
    return filelen;
}

// Test the 4 api calls, translate_barcode, translate_audio, add_item, delete_item also test parsing strings
void main() {

    // parse commands
    char new_string[100];
    assert(parse_command("add stuff", new_string) == CommandAdd);
    assert(strcmp(new_string, "stuff") == 0);
    assert(parse_command("remove stuff", new_string) == CommandRemove);
    assert(strcmp(new_string, "stuff") == 0);
    assert(parse_command("raspberries", new_string) == CommandNothing);
    assert(strcmp(new_string, "raspberries") == 0);
    assert(parse_command("salad dressing", new_string) == CommandNothing);
    assert(strcmp(new_string, "salad dressing") == 0);

    // parse ints
    assert(parse_number("add stuff", new_string) == 1);
    assert(strcmp(new_string, "add stuff") == 0);
    assert(parse_number("one stuff", new_string) == 1);
    assert(strcmp(new_string, "stuff") == 0);
    assert(parse_number("two stuff", new_string) == 2);
    assert(strcmp(new_string, "stuff") == 0);
    assert(parse_number("too things", new_string) == 2);
    assert(strcmp(new_string, "things") == 0);
    assert(parse_number("three stuff", new_string) == 3);
    assert(strcmp(new_string, "stuff") == 0);
    assert(parse_number("four stuff", new_string) == 4);
    assert(strcmp(new_string, "stuff") == 0);
    assert(parse_number("for stuff", new_string) == 4);
    assert(strcmp(new_string, "stuff") == 0);
    assert(parse_number("ten cows", new_string) == 10);
    assert(strcmp(new_string, "cows") == 0);

    char barcode_string[1000];
    char audio_string[1000];
    // Fruit Punch Juice Box,  8 - 6.75 fl oz boxes
    translate_barcode("028000521455", barcode_string);
    // Test barcode
    assert(strcmp(barcode_string, "Fruit Punch Juice Box,  8 - 6.75 fl oz boxes") == 0);
    printf("%s\n", barcode_string);

    long len = readFile();
    // how old is the Brooklyn Bridge
    translate_audio(buffer, len, audio_string);
    // Test audio
    assert(strcmp(audio_string, "how old is the Brooklyn Bridge") == 0);
    printf("%s\n", audio_string);

    // Test adding item
    assert(add_item("test", 1) == 1);

    // Test removing item
    assert(remove_item("test") == 1);

    printf("%s\n", "All tests passed!");
    return;
}

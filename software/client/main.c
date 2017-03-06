#include "client.h"
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

// Test the 4 api calls, translate_barcode, translate_audio, add_item, delete_item
void main() {
    char barcode_string[1000];
    char audio_string[1000];
    // Fruit Punch Juice Box,  8 - 6.75 fl oz boxes
    translate_barcode("028000521455", barcode_string);
    // Test barcode
    assert(strcmp(barcode_string, "Fruit Punch Juice Box,  8 - 6.75 fl oz boxes") == 0);

    long len = readFile();
    // how old is the Brooklyn Bridge
    translate_audio(buffer, len, audio_string);
    // Test audio
    assert(strcmp(audio_string, "how old is the Brooklyn Bridge") == 0);

    // Test adding item
    assert(add_item("test") == 1);

    // Test removing item
    assert(remove_item("test") == 1);

    printf("%s\n", "All tests passed!");
    return;
}
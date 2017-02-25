#include "client.h"
#include <stdio.h>
#include <stdlib.h>

char *buffer;

long readFile() {
    // Taken from http://stackoverflow.com/questions/22059189/read-a-file-as-byte-array
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

void main() {
    char barcode_string[1000];
    char audio_string[1000];
    //Fruit Punch Juice Box,  8 - 6.75 fl oz boxes
    translate_barcode("028000521455", barcode_string);
    printf("%s\n", barcode_string);
    long len = readFile();
    //how old is the Brooklyn Bridge
    translate_audio(buffer, len, audio_string);
    printf("%s\n", audio_string);
    return;
}
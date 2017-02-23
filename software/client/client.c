#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/fcntl.h>
#include "client.h"

#include "ipport.h"
#include "libport.h"
#include "osport.h"
#include "tcpport.h"

#define PORT 80
#define IP_ADDR "107.20.218.71"
#define MAX_HTTP_SIZE 10000
#define MAX_CHUNK 1024

int server_fd;
struct sockaddr_in server_info;
char request[MAX_HTTP_SIZE];
char response[MAX_HTTP_SIZE];
char body[MAX_HTTP_SIZE];

u_long inet_addr(char FAR * str);

// Set up socket file descriptor and connect to server
int create_connection() {
    // Set up socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Couldn't open socket");
        return -1;
    }

    // Set up information for server
    bzero(&server_info, sizeof(server_info));
    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(PORT);
    server_info.sin_addr.s_addr = inet_addr(IP_ADDR);

    // Connect to server
    if (connect(server_fd, (struct sockaddr*)&server_info, sizeof(server_info)) != 0)
    {
        perror("Couldn't connect to server");
        return -1;
    }
    return 1;
}

// Gets the response on the socket, returns total received bytes once connection dies
int reliable_receive() {
    int total_bytes = 0;
    // Loop over receiving until connection dies because it might be split into multiple packets
    while (1) {
        int bytes_received = recv(server_fd, response + total_bytes, MAX_HTTP_SIZE - total_bytes, 0);
        if (bytes_received == 0) {
            return total_bytes;
        } else if (bytes_received < 0) {
            perror("Error while receiving");
            return -1;
        } else {
            total_bytes += bytes_received;
        }
    }
    return 0;
}

// Create a request for a barcode
void create_barcode_request(char* barcode) {
    sprintf(request, "GET /barcode/%s HTTP/1.1\r\n\r\n", barcode);
}

// Grab the body of an http request
void parse_body() {
    // Ignore the starting newlines in body by adding 4
    char* position = strstr(response, "\r\n\r\n") + 4;
    strcpy(body, position);
}

int translate_barcode(char* barcode, char* resp)
{
	printf("starting connection\n");
    if(create_connection() < 0) {
    	sprintf(resp, "Could not connect to internet.");
    	return -1;
    }
    create_barcode_request(barcode);
    printf("sending values\n");
    if (send(server_fd, request, strlen(request), 0) < 0) {
        perror("Error while sending");
        sprintf(resp, "Could not connect to internet.");
        return -1;
    }
    printf("recievemingang values\n");
    int total_bytes = reliable_receive();
    printf("%i\n", total_bytes);
    response[total_bytes] = '\0';
    parse_body();
    printf("%s\n", response);
    close(server_fd);
    strcpy(resp, body);
    return 1;
}
int translate_audio(char* audio, char* resp) {
	sprintf(resp, "Chicken");
	return -1;
}
int add_item(char* item) {
	return -1;
}
int remove_item(char *item) {
	return -1;
}


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>

#define PORT 8080
#define IP_ADDR "127.0.0.1"
#define MAX_HTTP_SIZE 10000
#define MAX_CHUNK 1024

int server_fd;
struct sockaddr_in server_info;
char request[MAX_HTTP_SIZE];
char response[MAX_HTTP_SIZE];
char body[MAX_HTTP_SIZE];

// Set up socket file descriptor and connect to server
int create_connection() {
    printf("Trying to talk to server...\n");

    // Set up socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Couldn't open socket");
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
    }
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
            return;
        } else {
            total_bytes += bytes_received;
        }
    }
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

int main()
{
    create_connection();
    create_barcode_request("087684001165");
    if (send(server_fd, request, strlen(request), 0) < 0) {
        perror("Error while sending");
    }
    printf("Finished sending\n");
    int total_bytes = reliable_receive();
    response[total_bytes] = '\0';
    parse_body();
    printf("%s\n", body);
    close(server_fd);
}


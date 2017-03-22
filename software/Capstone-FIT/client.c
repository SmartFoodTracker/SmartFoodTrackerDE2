/** @file   client.c
 *  @brief  Routines to facilitate http requests to our server
 *
 *  Used to add items, remove items, translate barcodes and translate audio
 *  through http requests to a remote statically hosted server.
 *
 *  @author Andrew Bradshaw (abradsha), Kyle O'Shaughnessy (koshaugh)
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

// System routines
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/fcntl.h>
#include "ipport.h"
#include "libport.h"
#include "osport.h"
#include "tcpport.h"

// Web request routines
#include "client.h"

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

#define PORT            80
#define IP_ADDR         "13.56.5.40"
#define MAX_HTTP_SIZE   1000000
#define MAX_BODY_SIZE   1000
#define MAX_CHUNK       1024

/*****************************************************************************/
/* Globals                                                                   */
/*****************************************************************************/

int                 server_fd;
struct sockaddr_in  server_info;

// Header for barcode
static const char barcode_request[] = {"\
GET /barcode/%s HTTP/1.1\r\n\
Host: %s\r\n\
Connection: Close\r\n\r\n\
"};

// Header for audio
static const char audio_request[] = {"\
POST /speech HTTP/1.1\r\n\
Host: %s\r\n\
Connection: Close\r\n\
Content-Length: %ld\r\n\
Content-Type: audio/wav\r\n\r\n\
"};

// Header for adding
static const char add_request[] = {"\
PUT /1/inventory HTTP/1.1\r\n\
Host: %s\r\n\
Connection: Close\r\n\
Content-Length: %i\r\n\
Content-Type: application/json\r\n\r\n\
"};

// Header for deleting
static const char delete_request[] = {"\
DELETE /1/inventory/title/%s HTTP/1.1\r\n\
Host: %s\r\n\
Connection: Close\r\n\r\n\
"};

// Body for adding
static const char add_json[] = {"{\
\"title\": \"%s\",\
\"quantity\": 1,\
\"units\": \"whole\",\
\"timeAdded\": 1487568006,\
\"timeExpired\": 32326905600\
}"};

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

/**
 * @brief      Set up socket file descriptor and connect to server
 *
 * @return     1 if no error, otherwise -1 (socket or connection error)
 */
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
} // create_connection

/*****************************************************************************/

/**
 * @brief      Gets the response on the socket, returns total received bytes
 *             once connection dies
 *
 * @param      response  The response
 *
 * @return     { description_of_the_return_value }
 */
int reliable_receive(char* response) {
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
} // reliable_receive

/*****************************************************************************/

/**
 * @brief      Grab the body from an http response
 *
 * @param      response  The response
 * @param      body      The body
 */
void parse_body(char* response, char* body) {
    // Ignore the starting newlines in body by adding 4
    char* position = strstr(response, "\r\n\r\n") + 4;
    strcpy(body, position);
} // parse_body

/*****************************************************************************/

/**
 * @brief      Check if returned response says the request is valid
 *
 * @param      response  The response
 *
 * @return     { description_of_the_return_value }
 */
int good_response(char* response) {
    char* position = strstr(response, "200 OK");
    return position != NULL;
} // good_response

/*****************************************************************************/

/**
 * @brief      Creates a barcode request.
 *
 * @param      barcode  The barcode
 * @param      request  The request
 */
void create_barcode_request(char* barcode, char* request) {
    sprintf(request, barcode_request, barcode, IP_ADDR);
} // create_barcode_request

/*****************************************************************************/

/**
 * @brief      Convert a barcode to a plain-text item string
 *
 * @param      barcode  The barcode
 * @param      resp     The resp
 *
 * @return     { description_of_the_return_value }
 */
int translate_barcode(char* barcode, char* resp)
{
    char *request  = (char *) malloc(MAX_HTTP_SIZE * sizeof(char));
    char *response = (char *) malloc(MAX_HTTP_SIZE * sizeof(char));
    char *body     = (char *) malloc(MAX_BODY_SIZE * sizeof(char));
    if ((request  != NULL) &&
        (response != NULL) &&
        (body     != NULL)) {
        if(create_connection() < 0) {
            sprintf(resp, "Could not connect to internet.");
            if (request)
                free(request);

            if (response)
                free(response);

            if (body)
                free(body);
            return -1;
        }
        create_barcode_request(barcode, request);
        if (send(server_fd, request, strlen(request), 0) < 0) {
            perror("Error while sending");
            sprintf(resp, "Could not connect to internet.");
            if (request)
                free(request);

            if (response)
                free(response);

            if (body)
                free(body);
            return -1;
        }
        int total_bytes = reliable_receive(response);
        response[total_bytes] = '\0';
        parse_body(response, body);
        close(server_fd);
        strcpy(resp, body);
    }

    if (request)
        free(request);

    if (response)
        free(response);

    if (body)
        free(body);

    return 1;
} // translate_barcode

/*****************************************************************************/

/**
 * @brief      Create the request by generating the header and appending the
 *             audio file to it after
 *
 * @param      audio    The audio
 * @param[in]  len      The length
 * @param      request  The request
 *
 * @return     { description_of_the_return_value }
 */
long create_audio_request(char* audio, long len, char* request) {
    sprintf(request, audio_request, IP_ADDR, len);
    long header_length = strlen(request);
    memcpy(request+strlen(request),audio,len);
    return header_length;
} // create_audio_request

/*****************************************************************************/

/**
 * @brief      Convert a spoken word audio clip to plain-text tokens
 *
 * @param      audio         The audio
 * @param[in]  audio_length  The audio length
 * @param      resp          The resp
 *
 * @return     { description_of_the_return_value }
 */
int translate_audio(char* audio, long audio_length, char* resp) {
    char *request  = (char *) malloc(MAX_HTTP_SIZE * sizeof(char));
    char *response = (char *) malloc(MAX_HTTP_SIZE * sizeof(char));
    char *body     = (char *) malloc(MAX_BODY_SIZE * sizeof(char));
    if ((request  != NULL) &&
        (response != NULL) &&
        (body     != NULL)) {
        if(create_connection() < 0) {
            sprintf(resp, "Could not connect to internet.");
            if (request)
                free(request);

            if (response)
                free(response);

            if (body)
                free(body);
            return -1;
        }
        long header_length = create_audio_request(audio, audio_length, request);
        int sent_bytes = send(server_fd, request, header_length + audio_length, 0);
        if (sent_bytes < 0) {
            perror("Error while sending");
            sprintf(resp, "Could not connect to internet.");
            if (request)
                free(request);

            if (response)
                free(response);

            if (body)
                free(body);
            return -1;
        }
        int total_bytes = reliable_receive(response);
        response[total_bytes] = '\0';
        parse_body(response, body);
        close(server_fd);
        strcpy(resp, body);
    }

    if (request)
        free(request);

    if (response)
        free(response);

    if (body)
        free(body);

    return 1;
} // translate_audio

/*****************************************************************************/

/**
 * @brief      Create add request by generating the json we need then creating
               the header and returning the length of the total request size
 *
 * @param      item     The item
 * @param      request  The request
 *
 * @return     { description_of_the_return_value }
 */
int create_add_request(char* item, char* request) {
    char body[MAX_BODY_SIZE];
    sprintf(body, add_json, item);
    sprintf(request, add_request, IP_ADDR, (int)strlen(body));
    memcpy(request+strlen(request), body, strlen(body));
    return (int)strlen(request);
} // create_add_request

/*****************************************************************************/

/**
 * @brief      Adds an item to the FIT database
 *
 * @param      item  The item
 *
 * @return     { description_of_the_return_value }
 */
int add_item(char* item) {
    char request[MAX_HTTP_SIZE];
    char response[MAX_HTTP_SIZE];
    char body[MAX_BODY_SIZE];
    if(create_connection() < 0) {
        return -1;
    }
    long header_length = create_add_request(item, request);
    int sent_bytes = send(server_fd, request, header_length, 0);
    if (sent_bytes < 0) {
        perror("Error while sending");
        return -1;
    }
    int total_bytes = reliable_receive(response);
    response[total_bytes] = '\0';
    close(server_fd);
    return good_response(response);
} // add_item

/*****************************************************************************/

/**
 * @brief      Creates a delete request
 *
 * @param      item     The item
 * @param      request  The request
 */
void create_delete_request(char* item, char* request) {
    sprintf(request, delete_request, item, IP_ADDR);
} // create_delete_request

/*****************************************************************************/

// Remove item from our inventory based on a string

/**
 * @brief      Remove item from FIT database
 *
 * @param      item  The item
 *
 * @return     { description_of_the_return_value }
 */
int remove_item(char *item) {
    char request[MAX_HTTP_SIZE];
    char response[MAX_HTTP_SIZE];
    char body[MAX_BODY_SIZE];
    if(create_connection() < 0) {
        return -1;
    }
    create_delete_request(item, request);
    int sent_bytes = send(server_fd, request, strlen(request), 0);
    if (sent_bytes < 0) {
        perror("Error while sending");
        return -1;
    }
    int total_bytes = reliable_receive(response);
    response[total_bytes] = '\0';
    close(server_fd);
    return good_response(response);
} // remove_item

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

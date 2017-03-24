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
#include "client.h"

/*****************************************************************************/
/* Declarations                                                              */
/*****************************************************************************/

static int  create_connection();
static int  reliable_receive(char *pResponse);
static void parse_body(char *pResponse, char *pBody);
static int  good_response(char *pResponse);
static void create_barcode_request(char *pBarcodeString, char *pRequest);
static long create_audio_request(char *pAudioRecording,
                                 long  audioLengthBytes,
                                 char *pRequest);
static int  create_add_request(char *pItem, char *pRequest, int amount);
static void create_delete_request(char *pItem, char *pRequest);

/*****************************************************************************/
/* Globals                                                                   */
/*****************************************************************************/

int                 server_fd;
struct sockaddr_in  server_info;

// HTTP header for barcode
static const char barcode_request[] = {"\
GET /barcode/%s HTTP/1.1\r\n\
Host: %s\r\n\
Connection: Close\r\n\r\n\
"};

// HTTP header for audio
static const char audio_request[] = {"\
POST /speech HTTP/1.1\r\n\
Host: %s\r\n\
Connection: Close\r\n\
Content-Length: %ld\r\n\
Content-Type: audio/wav\r\n\r\n\
"};

// HTTP header for adding
static const char add_request[] = {"\
PUT /1/inventory HTTP/1.1\r\n\
Host: %s\r\n\
Connection: Close\r\n\
Content-Length: %i\r\n\
Content-Type: application/json\r\n\r\n\
"};

// HTTP header for deleting
static const char delete_request[] = {"\
DELETE /1/inventory/title/%s HTTP/1.1\r\n\
Host: %s\r\n\
Connection: Close\r\n\r\n\
"};

// JSON body for adding
static const char add_json[] = {"{\
\"title\": \"%s\",\
\"quantity\": %d,\
\"units\": \"whole\",\
\"timeAdded\": 1487568006,\
\"timeExpired\": 32326905600\
}"};

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

/**
 * @brief      Convert a barcode to a plain-text item string
 *
 * @param[in]     pBarcodeString  Barcode as a string
 * @param[inout]  pItemString     Item string representation, buffer must be
 *                                pre-allocated by caller
 *
 *
 * @return     1 if successful, 0 otherwise (pItemString will not be useful)
 */
int
translate_barcode(char *pBarcodeString, char *pItemString)
{
    int         total_bytes  = 0;
    FITRequest *pHttpRequest = (FITRequest *) malloc(sizeof(FITRequest));

    if (pHttpRequest != NULL)
    {
        if(create_connection() < 0)
        {
            sprintf(pItemString, "Could not connect to internet.");
            free(pHttpRequest);
            return 0;
        }

        create_barcode_request(pBarcodeString, pHttpRequest->pRequest);

        if (send(server_fd, pHttpRequest->pRequest, strlen(pHttpRequest->pRequest), 0) < 0)
        {
            perror("Error while sending");
            sprintf(pItemString, "Could not connect to internet.");
            free(pHttpRequest);
            return 0;
        }

        total_bytes = reliable_receive(pHttpRequest->pResponse);
        pHttpRequest->pResponse[total_bytes] = '\0';
        parse_body(pHttpRequest->pResponse, pHttpRequest->pBody);
        close(server_fd);
        strcpy(pItemString, pHttpRequest->pBody);
        free(pHttpRequest);
        return 1;
    }

    perror("HttpRequest malloc failed");
    return 0;
} // translate_barcode

/*****************************************************************************/

/**
 * @brief      Convert a spoken word audio clip to plain-text tokens
 *
 * @param[in]     pAudioRecording   Linear16 lossless audio recording
 * @param[in]     audioLengthBytes  Number of bytes in recording
 * @param[inout]  pItemString       Item string representation, buffer must be
 *                                  pre-allocated by caller
 *
 * @return     1 if successful, 0 otherwise (pItemString will not be useful)
 */
int
translate_audio(char *pAudioRecording, long audioLengthBytes, char *pItemString)
{
    int         total_bytes  = 0;
    FITRequest *pHttpRequest = (FITRequest *) malloc(sizeof(FITRequest));
    

    if (pHttpRequest != NULL)
    {
        if(create_connection() < 0)
        {
            sprintf(pItemString, "Could not connect to internet.");
            free(pHttpRequest);
            return 0;
        }

        long header_length = create_audio_request(pAudioRecording,
                                                  audioLengthBytes,
                                                  pHttpRequest->pRequest);
        int sent_bytes = send(server_fd,
                              pHttpRequest->pRequest,
                              header_length + audioLengthBytes,
                              0);
        if (sent_bytes < 0)
        {
            perror("Error while sending");
            sprintf(pItemString, "Could not connect to internet.");
            free(pHttpRequest);
            return 0;
        }

        total_bytes = reliable_receive(pHttpRequest->pResponse);
        pHttpRequest->pResponse[total_bytes] = '\0';
        parse_body(pHttpRequest->pResponse, pHttpRequest->pBody);
        close(server_fd);
        strcpy(pItemString, pHttpRequest->pBody);
        free(pHttpRequest);
        return 1;
    }

    perror("HttpRequest malloc failed");
    return 0;
} // translate_audio

/*****************************************************************************/

/**
 * @brief      Adds an item to the FIT database
 *
 * @param[in]  pItemString  The item to be added
 *
 * @return     1 if successful, 0 in case of error
 */
int
add_item(char *pItemString, int amount)
{
    int         retval          = 0;
    FITRequest *pHttpRequest    = (FITRequest *) malloc(sizeof(FITRequest));
    
    if (pHttpRequest != NULL)
    {
        if(create_connection() < 0)
        {
            free(pHttpRequest);
            return retval;
        }
        long header_length = create_add_request(pItemString, pHttpRequest->pRequest, amount);
        int sent_bytes = send(server_fd, pHttpRequest->pRequest, header_length, 0);
        if (sent_bytes < 0)
        {
            free(pHttpRequest);
            perror("Error while sending");
            return retval;
        }
        int total_bytes = reliable_receive(pHttpRequest->pResponse);
        pHttpRequest->pResponse[total_bytes] = '\0';
        close(server_fd);
        retval = good_response(pHttpRequest->pResponse);
        free(pHttpRequest);
    }
    else
    {
        perror("HttpRequest malloc failed");
    }

    return retval;
} // add_item

/*****************************************************************************/

/**
 * @brief      Remove item from FIT database
 *
 * @param[in]  pItemString  The item to be removed
 *
 * @return     1 if successful, o in case of error
 */
int
remove_item(char *pItemString)
{
    int         retval          = 0;
    FITRequest *pHttpRequest    = (FITRequest *) malloc(sizeof(FITRequest));
    
    if (pHttpRequest != NULL)
    {
        if(create_connection() < 0)
        {
            free(pHttpRequest);
            return retval;
        }
        create_delete_request(pItemString, pHttpRequest->pRequest);
        int sent_bytes = send(server_fd, pHttpRequest->pRequest, strlen(pHttpRequest->pRequest), 0);
        if (sent_bytes < 0)
        {
            perror("Error while sending");
            free(pHttpRequest);
            return retval;
        }
        int total_bytes = reliable_receive(pHttpRequest->pResponse);
        pHttpRequest->pResponse[total_bytes] = '\0';
        close(server_fd);
        free(pHttpRequest);
        retval = good_response(pHttpRequest->pResponse);
    }
    else
    {
        perror("HttpRequest malloc failed");
    }

    return retval;
} // remove_item

/*****************************************************************************/
/* Static Functions                                                          */
/*****************************************************************************/

/**
 * @brief      Set up socket file descriptor and connect to server
 *
 * @return     1 if successful, otherwise 0 (socket or connection error)
 */
static int
create_connection()
{
    // Setup socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Couldn't open socket");
        return 0;
    }

    // Set up information for server
    bzero(&server_info, sizeof(server_info));
    server_info.sin_family      = AF_INET;
    server_info.sin_port        = htons(FIT_PORT);
    server_info.sin_addr.s_addr = inet_addr(FIT_IP_ADDR);

    // Connect to server
    if (connect(server_fd, (struct sockaddr *) &server_info, sizeof(server_info)) != 0)
    {
        perror("Couldn't connect to server");
        return 0;
    }

    return 1;
} // create_connection

/*****************************************************************************/

/**
 * @brief      Gets the response on the socket, returns total received bytes
 *             once connection dies
 *
 * @param[inout]  pResponse  Response from server for particular request,
 *                           buffer must be pre-allocated by the caller
 *
 * @return     Total number of bytes received
 */
static int
reliable_receive(char *pResponse)
{
    int total_bytes     = 0;
    int bytes_received  = 0;

    // Loop over receiving until connection dies because it might be split into multiple packets
    while (1)
    {
        bytes_received = recv(server_fd, pResponse + total_bytes, FIT_MAX_HTTP_SIZE - total_bytes, 0);
        if (bytes_received == 0)
        {
            return total_bytes;
        }
        else if (bytes_received < 0)
        {
            perror("Error while receiving");
            return 0;
        }
        else
        {
            total_bytes += bytes_received;
        }
    }

    return 0;
} // reliable_receive

/*****************************************************************************/

/**
 * @brief      Grab the body from an http response
 *
 * @param[in]     pResponse  HTTP response that will be parsed
 * @param[inout]  pBody      Body of response, buffer must be pre-allcoated by
 *                           the caller
 */
static void
parse_body(char *pResponse, char *pBody)
{
    // Ignore the starting newlines in body by adding 4
    char *pPosition = strstr(pResponse, "\r\n\r\n") + 4;
    strcpy(pBody, pPosition);
} // parse_body

/*****************************************************************************/

/**
 * @brief      Check if returned response says the request is valid
 *
 * @param[in]  pResponse  HTTP response that is checked
 *
 * @return     1 if good, 0 otherwise
 */
static int
good_response(char *pResponse)
{
    return (strstr(pResponse, "200 OK") != NULL);
} // good_response

/*****************************************************************************/

/**
 * @brief      Creates a barcode request
 *
 * @param[in]     pBarcodeString  Barcode represented as a string
 * @param[inout]  pRequest  Request to be filled in, this buffer must be
 *                          pre-allocated by the caller
 */
static void
create_barcode_request(char *pBarcodeString, char *pRequest)
{
    sprintf(pRequest, barcode_request, pBarcodeString, FIT_IP_ADDR);
} // create_barcode_request

/*****************************************************************************/

/**
 * @brief      Create the request by generating the header and appending the
 *             audio file to it after
 *
 * @param[in]     pAudioRecording   Linear16 lossless audio recording
 * @param[in]     audioLengthBytes  Number of bytes in recording
 * @param[inout]  pRequest  Request to be filled in, this buffer must be
 *                          pre-allocated by the caller
 *
 * @return     The length of the resulting request in bytes
 */
static long
create_audio_request(char *pAudioRecording, long audioLengthBytes, char *pRequest)
{
    sprintf(pRequest, audio_request, FIT_IP_ADDR, audioLengthBytes);
    long header_length = strlen(pRequest);
    memcpy(pRequest + strlen(pRequest), pAudioRecording, audioLengthBytes);
    return header_length;
} // create_audio_request

/*****************************************************************************/

/**
 * @brief      Create add request by generating the json we need then creating
               the header and returning the length of the total request size
 *
 * @param[in]     pItemString  Item to be added
 * @param[in]     amount       amount of item to be added
 * @param[inout]  pRequest     Request to be filled in, this buffer must be
 *                             pre-allocated by the caller
 *
 * @return     The length of the resulting request in bytes
 */
static int
create_add_request(char *pItemString, char *pRequest, int amount)
{
    char body[FIT_MAX_BODY_SIZE];
    sprintf(body, add_json, pItemString, amount);
    sprintf(pRequest, add_request, FIT_IP_ADDR, (int)strlen(body));
    memcpy(pRequest+strlen(pRequest), body, strlen(body));
    return (int)strlen(pRequest);
} // create_add_request

/*****************************************************************************/

/**
 * @brief      Creates a delete request
 *
 * @param[in]     pItemString  Item to be deleted
 * @param[inout]  pRequest     Request to be filled in, this buffer must be
 *                             pre-allocated by the caller
 */
static void
create_delete_request(char *pItemString, char *pRequest)
{
    sprintf(pRequest, delete_request, pItemString, FIT_IP_ADDR);
} // create_delete_request

/*****************************************************************************/
/* End of File                                                               */
/*****************************************************************************/

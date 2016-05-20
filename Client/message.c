/*
    CITS3002 Project 2016
    Name:           Ammar Abu Shamleh, Pradyumn Vij, Goce Krlevski 
    Student number: 21469477
    Date:           d/m/2015
*/
#include "client.h"

/*
    sendMessage
    Parse the type of message required, direct user input appropriately
    @param host         Server address (IPV4, can be changed to IPV6 in main in the future)
    @param port         Server port
    @param action       Purpose of message
    @param file         File to upload / download / vouch
    @param certificate  Certificate to upload / sign
    @param minCircle    Minimum circle of trust required
    @param member       Member's name should be in circle of trust
    @return void        Maybe change to success / fail
*/
void parseRequest(char *host, char *port, int action, char *file, char *certificate,
                    int minCircle, char *member, bool memberRequested){
    //Establish connection with server
    SSL *ssl = secureConnection(host, port);
    
    //Perform user request
    switch(action) {
        case PUSH:
            sendFile(ssl, file, false);
            break;
        case PULL:
            //TODO: Perform file download
            break;
        case PUSH_CERT:
            sendFile(ssl, file true);
            break;
    }
    
    //Close connection to server
    closeConnection();
}

//Send file (indicated by filename) to server, using open socket descriptor 'sd'
void sendFile(SSL *ssl, char *filename, bool isCert) {
    int success;
    
    //First send the action number, so the server knows what to expect
    int action;
    if(!isCert) action = PUSH;
    else action = PUSH_CERT;
    success = SSL_write(ssl, &action, sizeof(int));
    if(success<0) {
        printf("Error sending action number\n");
        closeConnection();
        exit(EXIT_FAILURE);
    }
    
    //Now, send filename (this should be edited to include only the filename, and not the filepath)
    char *filename_to_server = strdup(filename);
    filename_to_server[strlen(filename_to_server)] = '\n'; //'\n' necessary for readLine() to work in server
    success = SSL_write(ssl, filename_to_server, sizeof(char) * strlen(filename_to_server));
    if(success<0) {
        printf("Error sending filename\n");
        closeConnection();
        exit(EXIT_FAILURE);
    }
    
    //Wait for server to send repsponse
    int response = readResponse(ssl);
    
    //Confirm response is a positive acknowledgment
    if(response != ACKNOWLEDGMENT) {
        printf("Error. Acknowledgment not received\n");
        printf("%d\n", response);
        closeConnection();
        exit(EXIT_FAILURE);
    }
    
    //Now, open file
    FILE *fpin = fopen(filename, "rb");
    if(fpin == NULL) {
        printf("Couldn't open file\n");
        closeConnection();
        exit(EXIT_FAILURE);
    }
    
    //Send file to server, one byte at a time
    char *buffer = (char *) malloc(sizeof(char));
    int n;
    n = fread(buffer, 1, sizeof(char), fpin);
    while(n > 0) {
        SSL_write(ssl, buffer, sizeof(char));
        n = fread(buffer, 1, 1, fpin);
    }
    
    //Free allocated memory and close resources
    fclose(fpin);
    free(buffer);
}

/*
 *Give function an SSL connection when expecting an int response from server
 *Function reads connection and returns int response (when it arrives)
 *http://stackoverflow.com/questions/16117681/sending-int-via-socket-from-java-to-c-strange-results
 */
int readResponse(SSL *ssl) {
    //Reads network-byte-order int off socket stream, and converts it to host-byte-order int before returning
    char intBuffer[4]; //Byte array for reading int
    int i=0;
    int receivedBytes = 0; //Counts number of bytes read (Java's writeInt() method sends 4-byte int, so we need to read 4 bytes)
    while(receivedBytes < 4){
        int readBytes = SSL_read(ssl, &intBuffer[i++], (sizeof intBuffer) - receivedBytes);
        receivedBytes += readBytes;
    }
    //Convert byte-array to int
    int receivedInt = *(int *) intBuffer;
    
    //Converts int to host-byte-order and returns
    return ntohl(receivedInt);
}

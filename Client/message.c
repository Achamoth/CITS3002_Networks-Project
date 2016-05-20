/*
    CITS3002 Project 2016
    Name:           Ammar Abu Shamleh, Pradyumn Vij, Goce Krlevski 
    Student number: 21469477
    Date:           d/m/2015
*/
#include "client.h"

/*
    sendAction
    
    Sends the type of message to the server.
    @param ssl      SSL session
    @param action   Type of message
*/
void sendAction(SSL *ssl, actionType action){
    if(SSL_write(ssl, &action, sizeof(int)) < 0){
        fprintf(stderr, "%s: Sending action type unsuccessful.\n", programName);
        closeConnection();
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: Action type sent successfully\n", programName);
    }
}

/*
    readResponse

    Give function an SSL connection when expecting an int response from server
    Function reads connection and returns int response (when it arrives)
    http://stackoverflow.com/questions/16117681/sending-int-via-socket-from-java-to-c-strange-results

    @param SSL * ssl    SSL session
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

/*
    sendFile

    Sends file from client to server.
    @param ssl          SSL session
    @param fileName     file to be uploaded
    @param isCert       String determine if certificate or file
    @param action       Type of message / action
*/
//Send file (indicated by filename) to server, using open socket descriptor 'sd'
void sendFile(SSL *ssl, char *fileName, char *isCert){
    //  Send filename
    char *serverFormattedName = strrchr(fileName, '/');
    //'\n' necessary for readLine() to work in server
    serverFormattedName[strlen(serverFormattedName)] = '\n';
    if(SSL_write(ssl, serverFormattedName, 
        sizeof(char)*strlen(serverFormattedName)) < 0){
        fprintf(stderr, "%s: Sending filename unsuccessful.\n", programName);
        closeConnection();
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: Filename sent successfully.\n", programName);
    }
    
    //  Wait for server response    
    //  Confirm response is a positive acknowledgment
    int response = readResponse(ssl);
    if(response != ACKNOWLEDGMENT){
        fprintf(stderr, "%s Error: Acknowledgment not received.\n",
            programName);
        fprintf(stderr, "Response received. %d\n", response);
        closeConnection();
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: Server acknowledgement recieved.\n", programName);
    }
    
    //  Create file pointer
    FILE *fp = fopen(fileName, "rb");
    if(fp == NULL) {
        perror("File Pointer");
        closeConnection();
        exit(EXIT_FAILURE);
    }
    
    //  Send file to server, one byte at a time
    char *buffer = (char *) malloc(sizeof(char));
    int bytes = fread(buffer, 1, sizeof(char), fp);
    while(bytes > 0) {
        SSL_write(ssl, buffer, sizeof(char));
        bytes = fread(buffer, 1, 1, fp);
    }
    
    //  Free allocated memory and close resources
    fclose(fp);
    free(buffer);
    free(serverFormattedName);
}

//Request that server send file (indicated by filename), using open socket descriptor 'sd'
void getFile(SSL *ssl, char *filename) {
    int success;
    
    //First send the action number, so the server knows what to expect
    int action = PULL;
    SSL_write(ssl, &action, sizeof(int));
    
    //Now, send file name
    char *filename_to_server = strdup(filename);
    filename_to_server[strlen(filename_to_server)] = '\n';
    success = SSL_write(ssl, filename_to_server, sizeof(char)*strlen(filename_to_server));
    if(success<0) {
        printf("Error sending action number\n");
        closeConnection();
        exit(EXIT_FAILURE);
    }
    
    //Wait for server to send response (indentifying whether or not it has the file)
    int response;
    response = readResponse(ssl);
    if(response != FILE_FOUND) {
        if(response == FILE_NOT_FOUND) printf("Error. Requested file doesn't exist on server\n");
        else printf("Unkown error occured. %d\n", response);
        closeConnection();
        exit(EXIT_FAILURE);
    }
    
    //Wait for server to report whether or not file meets trust requirements (DOESN'T WORK PROPERLY)
    response = readResponse(ssl);
    if(response != FILE_TRUSTWORTHY) {
        if(response == FILE_UNTRUSTWORTHY) printf("The file does not meet the specified trust requirements\n");
        else printf("Unknown error occured. %d\n", response);
        closeConnection();
        exit(EXIT_FAILURE);
    }
    
    //Wait for server to send file, and read it one byte at a time, writing each byte to file as it comes in
    FILE *fpout = fopen(filename, "wb");
    char *buffer = (char *) malloc(sizeof(char));
    int n;
    n = SSL_read(ssl, buffer, sizeof(char));
    while(n > 0) {
        fwrite(buffer, 1, sizeof(char), fpout);
        n = SSL_read(ssl, buffer, sizeof(char));
    }
    
    //Free all memory and close resources
    fclose(fpout);
    free(buffer);
}


/*
    sendMessage
    Parse the type of message required, direct user input appropriately
    
    @param  host         Server address (IPV4, can be changed to IPV6 in future)
    @param  port         Server port
    @param  action       Purpose of message
    @param  file         File to upload / download / vouch
    @param  certificate  Certificate to upload / sign
    @param  minCircle    Minimum circle of trust required
    @param  member       Member's name should be in circle of trust
    @return void         Maybe change to success / fail
*/
void parseRequest(char *host, char *port, actionType action, char *file, 
        char *certificate, int minCircle, char *member, bool memberRequested){
    //  Require host and port
    if(host == NULL || port == NULL){
        fprintf(stderr, "%s: Host and port required.\n", programName);
        usage();
    }
    //Establish connection with server
    SSL *ssl = secureConnection(host, port);
    
    //Perform user request
    switch(action) {
        case PUSH:{
            char * isCert = NULL;
            //  Check for file presence
            if(file == NULL && certificate == NULL){
                fprintf(stderr, "%s: File to upload not found.\n", programName);
                usage();
            }
            sendAction(ssl, PUSH);
            if(certificate == NULL){    //  Is a file
                isCert = "F";
                sendFile(ssl, file, isCert);
            }
            else{   //  Is a certificate
                isCert = "C";
                sendFile(ssl, certificate, isCert);
            }
            free(isCert);
            break;
        }
        case PULL:
            //Download file from server
            getFile(ssl, file);
            break;
        default:
            // Error action should be set
            fprintf(stderr, "%s: Action not set while parsing user request\n",
                programName);
            exit(EXIT_FAILURE);
    }
    
    //Close connection to server
    closeConnection();
}






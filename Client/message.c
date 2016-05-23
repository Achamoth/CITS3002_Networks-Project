/*
    CITS3002 Project 2016
    Name:           Ammar Abu Shamleh, Pradyumn Vij 
    Student number: 21521274, 21469477
    Date:           May 2016
*/
#include "client.h"

//Sends int so it can be read by Java's readInt() method
//http://stackoverflow.com/questions/3784263/converting-an-int-into-a-4-byte-char-array-c
static int sendInt(SSL *ssl, int n) {
    //Convert int to byte array
    unsigned char bytes[4];
    
    bytes[0] = (n >> 24) & 0xFF;
    bytes[1] = (n >> 16) & 0xFF;
    bytes[2] = (n >> 8) & 0xFF;
    bytes[3] = n & 0xFF;
    
    //Send byte array
    return SSL_write(ssl, bytes, sizeof(char) * 4);
}

/*
    sendAction
    
    Sends the type of message to the server.

    @param ssl      SSL session
    @param action   Type of message
*/
static void sendAction(SSL *ssl, actionType action){
    if(sendInt(ssl, action) < 0){
        fprintf(stderr, "%s: Sending action type unsuccessful.\n", programName);
        closeConnection();
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: Action type sent successfully\n", programName);
    }
}

/*
    sendFileString
    
    Sends the name of file that is being sent to the server using active SSL
    session.  File is formatted to remove path and newline character added to
    the end for Java interpretation.

    @param ssl      SSL session
    @param fileName String representing filename
*/
static void sendFileString(char *fileName, SSL *ssl){
    char *serverFormattedName = strrchr(strdup(fileName), '/');
    if(serverFormattedName == NULL) serverFormattedName = strdup(fileName);
    else serverFormattedName++;
    //'\n' necessary for readLine() to work in server
    serverFormattedName[strlen(serverFormattedName)] = '\n';
    if(SSL_write(ssl, serverFormattedName, 
        sizeof(char)*strlen(serverFormattedName)) < 0){
        fprintf(stderr, "%s: Sending filename/membername unsuccessful.\n", programName);
        closeConnection();
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: Filename/Membername sent successfully.\n", programName);
    }
}

/*
    readResponse

    Give function an SSL connection when expecting an int response from server
    Function reads connection and returns int response (when it arrives)
    http://stackoverflow.com/questions/16117681/sending-int-via-socket-from-java-to-c-strange-results

    @param SSL * ssl    SSL session
*/
static int readResponse(SSL *ssl) {
    //  Reads network-byte-order int off socket stream, and converts it to
    //  host-byte-order int before returning
    char intBuffer[4]; //Byte array for reading int
    //  Counts number of bytes read (Java's writeInt() method sends 4-byte int, 
    //  so we need to read 4 bytes)
    int receivedBytes = 0; 
    int i = 0;
    while(receivedBytes < 4){
        int readBytes = SSL_read(ssl, &intBuffer[i++], 
            (sizeof intBuffer) - receivedBytes);
        receivedBytes += readBytes;
    }
    //  Convert byte-array to int
    int receivedInt = *(int *) intBuffer;
    
    //  Converts int to host-byte-order and returns
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
static void sendFile(SSL *ssl, char *fileName){
    // Send file name to server
    sendFileString(fileName, ssl);
    
    //  Wait for server response    
    int response = readResponse(ssl);
    //  Confirm response is a positive acknowledgment
    if(response != ACKNOWLEDGMENT){
        fprintf(stderr, "%s Error: Acknowledgment not received.\n",
            programName);
        fprintf(stderr, "%s: Error: Acknowledgment not received.\n Response " 
            "fromOldTrusty Server: %d\n", programName, response);
        closeConnection();
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: Server acknowledgement recieved.\n"
            "\tSending file...\n",
            programName);
    }
    
    //  Create file pointer
    FILE *fp = fopen(fileName, "rb");
    if(fp == NULL) {
        perror("File Pointer");
        closeConnection();
        exit(EXIT_FAILURE);
    }
    
    //  Send file to server in 1024 byte chunks
    char buffer[1024];
    int written = 0;
    int read = 0;
    int bytes = (int) fread(buffer, sizeof(char), sizeof(buffer), fp);
    read += bytes;
    while(bytes > 0) {
        written += SSL_write(ssl, buffer, bytes);
        bytes = (int) fread(buffer, 1, sizeof(buffer), fp);
        read += bytes;
    }
    if(written != read){
        fprintf(stderr, "%s: File Transfer Error.\n", programName);
        closeConnection();
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: File Successfully sent.\n", programName);       
    }
    //  Free allocated memory and close resources
    fclose(fp);
}




//Request that server send file (indicated by filename)
//  Place in STDOUT, don't need to save
void getFile(SSL *ssl, char *fileName, int security, char* member) {
    // need to send security length
    if(sendInt(ssl, security) < 0) {
        fprintf(stderr, "%s: Sending required circle size unsuccessful.\n", programName);
        closeConnection();
        exit(EXIT_FAILURE);
    }
    
    //  Tell server whether or not there is a required member
    if(member == NULL) {
        //No member required.
        int toServer = MEMBER_NOT_REQUIRED;
        sendInt(ssl, toServer);
    }
    else {
        //Member required
        int toServer = MEMBER_REQUIRED;
        sendInt(ssl, toServer);
        //Now send name of required member
        sendFileString(member, ssl);
    }
    
    //  Send the name of file required
    sendFileString(fileName, ssl);
    
    //  Wait for server's response on availability of file
    int response;
    response = readResponse(ssl);
    if(response != FILE_FOUND) {
        if(response == FILE_NOT_FOUND){
            fprintf(stderr, "%s Error: Requested file cannot be found on" 
                "server\n", programName);
        }
        else{   // Inappropriate repsonse from Server
            fprintf(stderr, "%s: Unkown error occured.\n Response from "
                "OldTrusty Server: %d\n", programName, response);
        }
        closeConnection();
        exit(EXIT_FAILURE);
    }
    else{   // file found
        fprintf(stdout, "%s: File found on server.\n", programName);
        //  Wait for server to report on trustworthiness
        response = readResponse(ssl);
        if(response != FILE_TRUSTWORTHY) {
            if(response == FILE_UNTRUSTWORTHY){
                fprintf(stdout, "File does not meet your specified trust "
                    "requirements.\n");
            }
            else{   // Inappropriate response from Server
                fprintf(stderr, "%s: Unkown error occured.\n Response from "
                    "OldTrusty Server: %d\n", programName, response);
            }
            closeConnection();
            exit(EXIT_FAILURE);
        }
    }

    fprintf(stdout, "%s: File downloading...\n", programName);    
    //  Create file pointer
    FILE *fp = fopen(fileName, "wb");
    if(fp == NULL) {
        perror("File Pointer");
        closeConnection();
        exit(EXIT_FAILURE);
    }
    //  Wait for server to send file, and read it 1024 bytes at a time
    unsigned char buffer[1024];
    int written = 0;
    int read = 0;
    int bytes = SSL_read(ssl, buffer, sizeof(buffer));
    read += bytes;
    while(bytes > 0) {
        written += (int) fwrite(buffer, 1, bytes, fp);
        bytes = SSL_read(ssl, buffer, sizeof(char));
        read += bytes;
    }
    if(written != read){
        fprintf(stderr, "%s: File Transfer Error.\n", programName);
        closeConnection();
        exit(EXIT_FAILURE);
    }
    //  Check reason for closure
    if(bytes == SSL_ERROR_ZERO_RETURN){
        fprintf(stdout, "%s: File Download Complete.", programName); 
    }
    else{
        SSL_get_error(ssl, bytes); 
    }
    //Free all memory and close resources
    fclose(fp);
}

//Request that server vouch for specified file with specified certificate
void vouch(SSL *ssl, char *file, char *certificate) {
    //Send filename to server
    sendFileString(file, ssl);
    
    //Wait for server's repsonse (on whether or not it contains the file)
    int response = readResponse(ssl);
    if(response != FILE_FOUND) {
        if(response == FILE_NOT_FOUND){
            fprintf(stderr, "%s Error: Specified file cannot be found on"
                    "server\n", programName);
        }
        else{   // Inappropriate repsonse from Server
            fprintf(stderr, "%s: Unkown error occured.\n Response from "
                    "OldTrusty Server: %d\n", programName, response);
        }
        closeConnection();
        exit(EXIT_FAILURE);
    }
    
    //Next, send certificate name to server
    sendFileString(certificate, ssl);
    
    //Wait for server's response (on whether or not it contains the certificate)
    response = readResponse(ssl);
    if(response != FILE_FOUND) {
        if(response == FILE_NOT_FOUND){
            fprintf(stderr, "%s Error: Specified certificate cannot be found on"
                    "server\n", programName);
        }
        else{   // Inappropriate repsonse from Server
            fprintf(stderr, "%s: Unkown error occured.\n Response from "
                    "OldTrusty Server: %d\n", programName, response);
        }
        closeConnection();
        exit(EXIT_FAILURE);
    }
    
    //TODO: Might want to add some stuff here later. I'll leave it for now
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
            //  Check for file presence
            if(file == NULL){
                fprintf(stderr, "%s: File to upload not found.\n", programName);
                usage();
            }
            sendAction(ssl, PUSH);
            sendFile(ssl, file);
            break;
        }
        case PULL:
            //Download file from server
            sendAction(ssl, PULL);
            getFile(ssl, file, minCircle, member);
            break;
        case PUSH_CERT:
            //Upload certificate to server
            if(certificate == NULL) {
                fprintf(stderr, "%s: Certificate to upload not found.\n", 
                    programName);
                usage();
            }
            sendAction(ssl, PUSH_CERT);
            sendFile(ssl, certificate);
            break;
        case VOUCH:
            //Vouch for specified file with specified certificate
            if(file == NULL) {
                fprintf(stderr, "%s: File to vouch for not found.\n", programName);
                usage();
            }
            else if(certificate == NULL) {
                fprintf(stderr, "%s: Certificate to vouch with not found.\n", programName);
                usage();
            }
            sendAction(ssl, VOUCH);
            vouch(ssl, file, certificate);
            break;
        default:
            // Error action should be set
            fprintf(stderr, "%s: Action not set while parsing user request\n",
                programName);
            exit(EXIT_FAILURE);
    }
    fprintf(stdout, "%s: Exiting Client...\n", programName);
    //Close connection to server
    closeConnection();
}

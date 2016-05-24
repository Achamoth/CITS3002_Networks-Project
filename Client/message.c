/*
    CITS3002 Project 2016
    Name:           Ammar Abu Shamleh, Pradyumn Vij 
    Student number: 21521274, 21469477
    Date:           May 2016
*/
#include "client.h"


/*
    readByteArray

    @param  bytes       Expected number of bytes to receive in byte array
*/
static char *readByteArray(int numBytes){
    char *buffer = malloc(numBytes);
    int receivedBytes = 0;
    int i = 0;
    while(receivedBytes < numBytes){
        int readBytes = 
            SSL_read(ssl, buffer + i,(sizeof buffer) - receivedBytes);
        receivedBytes += readBytes;
        ++i;
    }
}


/*
    getRSAfrom

    @param  privateKeyFile  Location of the PEM format private key
    @return     RSA struct containing the private key, extracted from PEM
*/
static RSA *getRSAfrom(char *privateKeyFile){
    File *fp = fopen(privateKeyFile, "rb");
    if(fp == NULL){
        perror("Extract RSA from PEM file");
        exit(EXIT_FAILURE);
    }
    else{
        RSA *rsa = RSA_new();
        if(rsa == NULL){
            fprintf(stderr, "%s Error: Failed to create RSA.\n");
            exit(EXIT_FAILURE);
        }
        PEM_read_RSAPrivateKey(fp, &rsa, NULL, NULL);
    }
    return rsa;
}


/*
    handleChallenge

    Helper function when vouching, Server challenges when vouching for file
    by sending a randomly generated number or code.
    This code is then encrytped using the public key that the Server has,
    corresponding to this client.
    The client decrypts with its private key, makes the appropriate modification
    then re-encrypts with the private key and sends back to Server to prove
    that the client has the private key and is indeed holding the private key.
*/
static void handleChallenge(){
    //  Get the RSA
    RSA *privateKey = getRSAfrom(PRIVATE_KEY);
    // Size of key or RSA
    int keyLength = RSA_size(privateKey);

    // Assign max encrytped memory block
    unsigned char *encrypted = calloc(keyLength, sizeof(char));
    // Assign memory block to pass plain text to
    unsigned char *decrypted = malloc(keyLength+1);
    // Clear memory block
    memset(decrypted, 0, keyLength+1);

    // Read incoming data.
    int error;
    if(error = SSL_read(ssl, encrypted, 30) < 0){
        fprintf(stderr, "%s Error: Failed to receive vouch challenge\n");
        SSL_get_error(error);
    }
    else{
        fprintf(stdout, "%s: Received vouch challenge from Server.\n");
    }

    // Decrypt challenge
    if(RSA_private_decrypt(keyLength, encrypted, decrypted, privateKey,
        RSA_PKCS1_PADDING) < 0){
        fprintf(stderr, "%s Error: Could not decrypt Server's vouch challenge\n");
        closeConnection();
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: Decrypted Server vouch challenge.\n");
    }

    // Add 1 or do some other change to number or long or uint64
    int challengeNumber = *(int*) decrypted;
    challengeNumber = ntohl(challengeNumber);
    ++challengeNumber;
    // Clear prior contents
    memset(decrypted, 0, keyLength+1);
    memset(encrypted, 0, keyLength);

    // Convert int to character pointer
    decrypted = (char*)&challengeNumber;
    // Encrypt challenge answer
    if(RSA_private_encrypt(keyLength, decrypted, encrypted, privateKey, RSA_PKCS1_PADDING) < 0){
        fprintf(stderr, "%s Error: Encrypting vouch reply failed.\n");
        closeConnection();
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: Encrypted vouch reply.\n");
    }

    // Send Vouch Challenge response
    if(SSL_write(ssl, encrypted, keyLength) < 0){
        fprintf(stderr, "%s Error: Sending vouch challenge response failed.",
            programName);
    }
    else{
        fprintf(stdout, "%s: Vouch Challenge sent successfully.\n", programName);
    }

    RSA_free(privateKey);
    free(encrypted);
    free(decrypted);
}

/*
    sendAction
    
    Sends the type of message to the Server.

    @param ssl      SSL session
    @param action   Type of message
*/
static void sendAction(SSL *ssl, actionType action){
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
    sendFileString
    
    Sends the name of file that is being sent to the Server using active SSL
    session.  File is formatted to remove path and newline character added to
    the end for Java interpretation.

    @param ssl      SSL session
    @param fileName String representing filename
*/
static void sendFileString(char *fileName, SSL *ssl){
    char *ServerFormattedName = strrchr(strdup(fileName), '/');
    if(ServerFormattedName == NULL) ServerFormattedName = strdup(fileName);
    else ServerFormattedName++;
    //'\n' necessary for readLine() to work in Server
    ServerFormattedName[strlen(ServerFormattedName)] = '\n';
    if(SSL_write(ssl, ServerFormattedName, 
        sizeof(char)*strlen(ServerFormattedName)) < 0){
        fprintf(stderr, "%s: Sending filename unsuccessful.\n", programName);
        closeConnection();
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: Filename sent successfully.\n", programName);
    }
}

/*
    readResponse

    Give function an SSL connection when expecting an int response from Server
    Function reads connection and returns int response (when it arrives)
    http://stackoverflow.com/questions/16117681/sending-int-via-socket-from-java-to-c-strange-results

    @param SSL * ssl    SSL session
    @return int         Response of a preprocessor defined or ENUM form.
*/
static int readResponse(SSL *ssl) {
    //  Reads network-byte-order int off socket stream, and converts it to
    //  host-byte-order int before returning
    char intBuffer[4]; //Byte array for reading in
    //  Counts number of bytes read (Java's writeInt() method sends 4-byte int, 
    //  so we need to read 4 bytes)
    int receivedBytes = 0;
    int i = 0;
    while(receivedBytes < 4){
        int readBytes = SSL_read(ssl, &intBuffer[i++], 
            sizeof(intBuffer) - receivedBytes);
        receivedBytes += readBytes;
    }
    //  Convert byte-array to int
    int receivedInt = *(int *) intBuffer;
    
    //  Converts int to host-byte-order and returns
    return ntohl(receivedInt);
}

/*
    sendFile

    Sends file from client to Server.

    @param ssl          SSL session
    @param fileName     file to be uploaded
    @param isCert       String determine if certificate or file
    @param action       Type of message / action
*/
static void sendFile(SSL *ssl, char *fileName){
    // Send file name to Server
    sendFileString(fileName, ssl);
    
    //  Wait for Server response    
    int response = readResponse(ssl);
    //  Confirm response is a positive acknowledgment
    if(response != ACKNOWLEDGMENT){
        fprintf(stderr, "%s: Error: Acknowledgment not received.\n Response " 
            "from Server: %d\n", programName, response);
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
    
    //  Send file to Server in 1024 byte chunks
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




//Request that Server send file (indicated by filename)
//  Place in STDOUT, don't need to save
void getFile(SSL *ssl, char *fileName, int security) {
    // need to send security length
    if(SSL_write(ssl, &security, sizeof(int)) < 0) {
        fprintf(stderr, "%s: Sending required circle size unsuccessful.\n", programName);
        closeConnection();
        exit(EXIT_FAILURE);
    }
    
    //  Send the name of file required
    sendFileString(fileName, ssl);
    
    //  Wait for Server's response on availability of file
    int response;
    response = readResponse(ssl);
    if(response != FILE_FOUND) {
        if(response == FILE_NOT_FOUND){
            fprintf(stderr, "%s Error: Requested file cannot be found on" 
                "Server\n", programName);
        }
        else{   // Inappropriate repsonse from Server
            fprintf(stderr, "%s Error: Unkown error.\n Response from "
                "Server: %d\n", programName, response);
        }
        closeConnection();
        exit(EXIT_FAILURE);
    }
    else{   // file found
        fprintf(stdout, "%s: File found on Server.\n", programName);
        //  Wait for Server to report on trustworthiness
        response = readResponse(ssl);
        if(response != FILE_TRUSTWORTHY) {
            if(response == FILE_UNTRUSTWORTHY){
                fprintf(stdout, "%s Error: File does not meet your specified trust "
                    "requirements.\n", programName);
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
    //  Wait for Server to send file, and read it 1024 bytes at a time
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
        SSL_get_error(ssl, bytes);
        closeConnection();
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: File Download Complete.", programName); 
    }
    //Free all memory and close resources
    fclose(fp);
}

//Request that Server vouch for specified file with specified certificate
void vouch(SSL *ssl, char *file, char *certificate) {
    //Send filename to Server
    sendFileString(file, ssl);
    
    //Wait for Server's repsonse (on whether or not it contains the file)
    int response = readResponse(ssl);
    if(response != FILE_FOUND) {
        if(response == FILE_NOT_FOUND){
            fprintf(stderr, "%s Error: Specified file cannot be found on"
                    "Server\n", programName);
        }
        else{   // Inappropriate repsonse from Server
            fprintf(stderr, "%s Error: Unkown error.\n Response from "
                    "OldTrusty Server: %d\n", programName, response);
        }
        closeConnection();
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: File found on Server.\n", programName);
    }
    
    //Next, send certificate name to Server
    sendFileString(certificate, ssl);
    
    //Wait for Server's response (on whether or not it contains the certificate)
    response = readResponse(ssl);
    if(response != FILE_FOUND) {
        if(response == FILE_NOT_FOUND){
            fprintf(stderr, "%s Error: Specified certificate cannot be found on"
                    "Server\n", programName);
        }
        else{   // Inappropriate repsonse from Server
            fprintf(stderr, "%s Error: Unkown error.\n Response from "
                    "OldTrusty Server: %d\n", programName, response);
        }
        closeConnection();
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: Certificate exists on Server.\n", programName);
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
    //Establish connection with Server
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
            //Download file from Server
            sendAction(ssl, PULL);
            getFile(ssl, file, minCircle);
            break;
        case PUSH_CERT:
            //Upload certificate to Server
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
    //Close connection to Server
    closeConnection();
}

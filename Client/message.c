/*
    CITS3002 Project 2016
    Name:           Ammar Abu Shamleh, Pradyumn Vij 
    Student number: 21521274, 21469477
    Date:           May 2016
*/
#include "client.h"
#define BUFFER_SIZE 65536

/*
 readByteArray
 
 @param  bytes       Expected number of bytes to receive in byte array
 */
/*static char *readByteArray(int numBytes, SSL *ssl){
    char *buffer = malloc(numBytes);
    int receivedBytes = 0;
    int i = 0;
    while(receivedBytes < numBytes){
        int readBytes =
        SSL_read(ssl, buffer + i,(sizeof buffer) - receivedBytes);
        receivedBytes += readBytes;
        ++i;
    }
    return buffer;
}*/

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
 getRSAfrom
 
 @param  privateKeyFile  Location of the PEM format private key
 @return     RSA struct containing the private key, extracted from PEM
 */
static RSA *getRSAfrom(char *privateKeyFile){
    RSA *rsa = NULL;
    FILE *fp = fopen(privateKeyFile, "rb");
    if(fp == NULL){
        perror("Extract RSA from PEM file");
        exit(EXIT_FAILURE);
    }
    else{
        rsa = RSA_new();
        if(rsa == NULL){
            fprintf(stderr, "%s Error: Failed to create RSA.\n", programName);
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
static void handleChallenge(SSL *ssl, char *keyName){
    //  Get the RSA
    RSA *privateKey = getRSAfrom(keyName);
    // Size of key or RSA
    int keyLength = RSA_size(privateKey);
    
    // Assign max encrytped memory block
    unsigned char encrypted[keyLength];
    // Assign memory block to pass plain text to
    unsigned char decrypted[4];
    
    //Read length of cipher (sent by Server)
    int cipherLength = readResponse(ssl);
    
    //Read cipher byte by byte off connection
    for(int i=0; i<cipherLength; i++) {
        SSL_read(ssl, &encrypted[i], 1);
    }
    
    // Decrypt challenge
    if(RSA_private_decrypt(keyLength, encrypted, decrypted, privateKey,
                           RSA_PKCS1_PADDING) < 0){
        fprintf(stderr, "%s Error: Could not decrypt Server's vouch challenge\n", programName);
        closeConnection();
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: Decrypted Server vouch challenge.\n", programName);
    }
    
    // Add 1 or do some other change to number or long or uint64
    int challengeNumber = *(int*) decrypted;
    challengeNumber = ntohl(challengeNumber);
    ++challengeNumber;
    
    // Convert int to character pointer
    convertToBytes(htonl(challengeNumber), decrypted);
    
    // Encrypt challenge answer
    unsigned char encryptedModified[keyLength];
    if(RSA_private_encrypt(4, decrypted, encryptedModified, privateKey, RSA_PKCS1_PADDING) < 0){
        fprintf(stderr, "%s Error: Encrypting vouch reply failed.\n" ,programName);
        closeConnection();
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: Encrypted vouch reply.\n", programName);
    }
    
    //Send keyLength
    sendInt(ssl, keyLength);
    
    // Send Vouch Challenge response
    if(SSL_write(ssl, encryptedModified, keyLength) < 0){
        fprintf(stderr, "%s Error: Sending vouch challenge response failed.",
                programName);
    }
    else{
        fprintf(stdout, "%s: Vouch Challenge sent successfully.\n", programName);
    }
    
    RSA_free(privateKey);
}

//Converts an int to a 4 byte character array
void convertToBytes(int n, unsigned char bytes[]) {
    bytes[0] = n;
    bytes[1] = n>>8;
    bytes[2] = n>>16;
    bytes[3] = n>>24;
}

/*
    sendAction
    
    Sends the type of message to the server.

    @param ssl      SSL session
    @param action   Type of message
*/
static void sendAction(SSL *ssl, actionType action){
    if(sendInt(ssl, action) < 0){
        fprintf(stderr, "%s: Error: Sending action type unsuccessful.\n", programName);
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
        fprintf(stderr, "%s: Error: Sending filename/membername unsuccessful.\n", programName);
        closeConnection();
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: Filename/Membername sent successfully.\n", programName);
    }
}

/*
    sendFile

    Sends file from client to server.

    @param ssl          SSL session
    @param fileName     file to be uploaded
    @param isCert       String determine if certificate or file
    @param action       Type of message / action
*/
static void sendFile(SSL *ssl, char *fileName, bool isCert){
    //  Create file pointer
    FILE *fp = fopen(fileName, "rb");
    if(fp == NULL) {
        perror("File Pointer");
        sendInt(ssl, CLIENT_INVALID_FILE); //ERROR CHECKING
        closeConnection();
        exit(EXIT_FAILURE);
    }
    
    //Tell server that we have the file (i.e. filename wasn't invalid)
    sendInt(ssl, CLIENT_VALID_FILE); //ERROR CHECKING
    
    // Send file name to server
    sendFileString(fileName, ssl);
    
    //  Wait for server response    
    int response = readResponse(ssl);
    //  Confirm response is a positive acknowledgment
    if(response != ACKNOWLEDGMENT){
        fprintf(stderr, "%s Error: Acknowledgment not received.\n",
            programName);
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
    
    if(isCert) {
        //Send server size of file
        fseek(fp, 0, SEEK_END);
        int size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        sendInt(ssl, size);
    }
    
    //  Send file to server in chunks
    char buffer[BUFFER_SIZE];
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
        fprintf(stderr, "%s: Error: File Transfer Error.\n", programName);
        closeConnection();
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: File Successfully sent.\n", programName);       
    }
    //  Free allocated memory and close resources
    fclose(fp);
    
    //Now, if file is a certificate, some extra control work needs to be done
    if(isCert) {
        //First, server will report whether or not certificate with 'file' as name already exists
        response = readResponse(ssl);
        
        //If no such certificate already exists, then simply return
        if(response == CERT_NO_CONFLICT) {
            fprintf(stdout, "%s: Certificate doesn't already exist on Server. Saved successfully\n", programName);
        }
        
        //Otherwise, server will issue a public key challenge
        else if (response == CERT_CONFLICT) {
            //Server will now issue cryptographic challenge
            fprintf(stdout, "%s: Certificate already exists on Server. Server issuing public key challenge to ensure client owns original certificate before replacing\n", programName);
            //First, create path to appropriate private key (assume it's in "keys" folder)
            char keyPath[100];
            char *key = strdup(fileName);
            int keyLength = strlen(key);
            key[keyLength-1] = 'y'; key[keyLength-2] = 'e'; key[keyLength-3] = 'k';
            sprintf(keyPath, "keys/%s", key);
            
            //Now respond to challenge
            handleChallenge(ssl, keyPath);
            free(key);
            
            //Now Server will inform the client whether or not it passed the challenge
            response = readResponse(ssl);
            if(response != PASS_CHALLENGE) {
                if(response == FAIL_CHALLENGE) {
                    fprintf(stderr, "%s Error: Failed to pass Server's cryptographic challenge. Authorization to replace certificate not given.\n", programName);
                    closeConnection();
                    exit(EXIT_FAILURE);
                }
                else { //Inappropriate response from Server
                    fprintf(stderr, "%s Error: Unknown error.\n Response from Server: %d\n", programName, response);
                    closeConnection();
                    exit(EXIT_FAILURE);
                }
            }
            
            fprintf(stdout, "%s: Successfully passed Server's cryptographic challenge. Authorization to replace certificate given\n", programName);
        }
        
        else {
            fprintf(stdout, "%s: Error: Unknown error.\n Response form Server: %d\n", programName, response);
        }
    }
}




//Request that server send file (indicated by filename)
//  Place in STDOUT, don't need to save
void getFile(SSL *ssl, char *fileName, int security, char* member) {
    // need to send security length
    if(sendInt(ssl, security) < 0) {
        fprintf(stderr, "%s: Error: Sending required circle size unsuccessful.\n", programName);
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
            fprintf(stderr, "%s: Error: Unkown error.\n Response from "
                "Server: %d\n", programName, response);
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
                fprintf(stderr, "%s: Error: Unkown error.\n Response from "
                    "Server: %d\n", programName, response);
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
    unsigned char buffer[BUFFER_SIZE];
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
        fprintf(stderr, "%s: Error: File Transfer Error.\n", programName);
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
            fprintf(stderr, "%s: Error: Unkown error.\n Response from "
                    "Server: %d\n", programName, response);
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
            fprintf(stderr, "%s Error: Specified certificate cannot be found on "
                    "server\n", programName);
        }
        else{   // Inappropriate repsonse from Server
            fprintf(stderr, "%s: Error: Unkown error.\n Response from "
                    "Server: %d\n", programName, response);
        }
        closeConnection();
        exit(EXIT_FAILURE);
    }
    
    //Server will now issue cryptographic challenge
    //First, create path to appropriate private key (assume it's in "keys" folder)
    char keyPath[100];
    char *key = strdup(certificate);
    int keyLength = strlen(key);
    key[keyLength-1] = 'y'; key[keyLength-2] = 'e'; key[keyLength-3] = 'k';
    sprintf(keyPath, "keys/%s", key);
            
    //Now respond to challenge
    handleChallenge(ssl, keyPath);
    free(key);
    
    //Now Server will inform the client whether or not it passed the challenge
    response = readResponse(ssl);
    if(response != PASS_CHALLENGE) {
        if(response == FAIL_CHALLENGE) {
            fprintf(stderr, "%s Error: Failed to pass Server's cryptographic challenge. Authorization to vouch not given.\n", programName);
            closeConnection();
            exit(EXIT_FAILURE);
        }
        else { //Inappropriate response from Server
            fprintf(stderr, "%s Error: Unknown error.\n Response from Server: %d\n", programName, response);
            closeConnection();
            exit(EXIT_FAILURE);
        }
    }
    
    fprintf(stdout, "%s: Successfully passed Server's cryptographic challenge. Authorization to vouch given\n", programName);
    
    fprintf(stdout, "%s: Successfully vouched for \"%s\" with \"%s\"\n", programName, file, certificate);
}

//Request that server send list of files that it contains, along with each file's level of protection
void list(SSL *ssl) {
    printf("\n");
    //Server will start by sending the number of files
    int nFiles = readResponse(ssl);
    printf("%d files on server\n\n", nFiles);
    
    //Server will then send data for each file
    for(int i=0; i<nFiles; i++) {
        char filename[260]; //260 characters is the max filename length
        char recvChar;
        int circleSize;
        int j = 0;
        //Server will first send the length of the file
        int nameLength = readResponse(ssl);
        //Server will then send the name of the file
        for(int x=0; x<nameLength; x++) {
            SSL_read(ssl, &recvChar, sizeof(char));
            filename[j++] = recvChar;
        }
        filename[j] = '\0';
        //Print filename
        printf("%s\n", filename);
        //Now, server will send the size of the circle of trust
        circleSize = readResponse(ssl);
        //Print circle size
        printf("Circle size %d : ", circleSize);
        //Now, server will send list of names used to
        for(int k=0; k<circleSize; k++) {
            if(k != 0) printf(", ");
            char curName[30];
            j=0;
            //Read name length
            nameLength = readResponse(ssl);
            //Read current name
            for(int x=0; x<nameLength; x++) {
                SSL_read(ssl, &recvChar, sizeof(char));
                curName[j++] = recvChar;
            }
            curName[j] = '\0';
            //Print voucher's name
            printf("%s", curName);
        }
        printf("\n\n");
    }
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
            sendFile(ssl, file, false);
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
            sendFile(ssl, certificate, true);
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
        case LIST:
            //Request server to list all items, and their level of protection
            sendAction(ssl, LIST);
            list(ssl);
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

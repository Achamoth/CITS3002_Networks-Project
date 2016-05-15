//Written by Ammar Abu Shamleh

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#define UPLOAD 1
#define DOWNLOAD 2
#define ACKNOWLEDGMENT 10
#define FILE_NOT_FOUND 15
#define FILE_FOUND 16

#define PORT 8889

void sendFile(int sd, char *filename);
void downloadFile(int sd, char *filename);
int readResponse(int sd);

void error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    //Required variables for socket connection
    int port = PORT;
    struct sockaddr_in server_address;
    struct hostent *server;
    
    //Allocate memory for socket and return socket descriptor
    int sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sd < 0) {
        perror("Error: ");
        exit(EXIT_FAILURE);
    }
    
    //Ask user to input name of server
    printf("Please enter hostname of server: ");
    char hostname[256];
    scanf("%s", hostname);
    
    //Acquire host entity data by searching for host name
    server = gethostbyname(hostname);
    if(server == NULL) {
        printf("Error: No such host\n");
        exit(EXIT_FAILURE);
    }
    
    //Set up server_address struct
    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
    server_address.sin_port = htons(port);
    
    //Connect to server using 'sd' socket descriptor and 'server_address' struct
    if(connect(sd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        error("Error connecting");
    }
    
    if(!strcmp(argv[1], "upload")) {
        //User wants to upload file. Send file to server, and have server save it
        sendFile(sd, strdup(argv[2]));
    }
    else if(!strcmp(argv[1], "download")) {
        //User wants to download file. Send filename to server, and have server send requested file
        downloadFile(sd, strdup(argv[2]));
    }
    
    //Close socket connection
    shutdown(sd, SHUT_RDWR);
    close(sd);
    
    //Close program
    return 0;
}

//Send file (indicated by filename) to server, using open socket descriptor 'sd'
void sendFile(int sd, char *filename) {
    int success;
    
    //First send the action number, so the server knows what to expect
    int action = UPLOAD;
    success = write(sd, &action, sizeof(int));
    if(success<0) {
        printf("Error sending action number\n");
        exit(EXIT_FAILURE);
    }
    
    //Now, send filename (this should be edited to include only the filename, and not the filepath)
    char *filename_to_server = strdup(filename);
    filename_to_server[strlen(filename_to_server)] = '\n'; //'\n' necessary for readLine() to work in server
    success = write(sd, filename_to_server, sizeof(char) * strlen(filename_to_server));
    if(success<0) {
        printf("Error sending filename\n");
        exit(EXIT_FAILURE);
    }
    
    //Wait for server to send repsponse
    int response = readResponse(sd);
    
    //Confirm response is a positive acknowledgment
    if(response != ACKNOWLEDGMENT) {
        printf("Error. Acknowledgment not received\n");
        printf("%d\n", response);
    }
    
    //Now, open file
    FILE *fpin = fopen(filename, "rb");
    if(fpin == NULL) {
        printf("Couldn't open file\n");
        exit(EXIT_FAILURE);
    }
    
    //Send file to server, one byte at a time
    char *buffer = (char *) malloc(sizeof(char));
    int n;
    n = fread(buffer, 1, sizeof(char), fpin);
    while(n > 0) {
        write(sd, buffer, sizeof(char));
        n = fread(buffer, 1, 1, fpin);
    }
    
    //Free allocated memory and close resources
    fclose(fpin);
    free(buffer);
}

//Request that server send file (indicated by filename), using open socket descriptor 'sd'
void downloadFile(int sd, char *filename) {
    int success;
    
    //First send the action number, so the server knows what to expect
    int action = DOWNLOAD;
    write(sd, &action, sizeof(int));
    
    //Now, send file name
    char *filename_to_server = strdup(filename);
    filename_to_server[strlen(filename_to_server)] = '\n';
    success = write(sd, filename_to_server, sizeof(char)*strlen(filename_to_server));
    if(success<0) {
        printf("Error sending action number\n");
    }
    
    //TODO: Wait for server to send response (indentifying whether or not it has the file)
    int response;
    response = readResponse(sd);
    if(response != FILE_FOUND) {
        if(response == FILE_NOT_FOUND) printf("Error. Requested file doesn't exist on server\n");
        else printf("Unkown error occured. %d\n", response);
        exit(EXIT_FAILURE);
    }
    
    //Wait for server to send file, and read it one byte at a time, writing each byte to file as it comes in
    FILE *fpout = fopen(filename, "wb");
    char *buffer = (char *) malloc(sizeof(char));
    int n;
    n = read(sd, buffer, sizeof(char));
    while(n > 0) {
        fwrite(buffer, 1, sizeof(char), fpout);
        n = read(sd, buffer, sizeof(char));
    }
    
    //Free all memory and close resources
    fclose(fpout);
    free(buffer);
}

/*
 *Give function a socket descriptor when expecting an int response from server
 *Function reads socket descriptor and returns int response (when it arrives)
 */
int readResponse(int sd) {
    //Reads network-byte-order int off socket stream, and converts it to host-byte-order int before returning
    char intBuffer[4]; //Byte array for reading int
    int i=0;
    int receivedBytes = 0; //Counts number of bytes read (Java's writeInt() method sends 4-byte int, so we need to read 4 bytes)
    while(receivedBytes < 4){
        int readBytes = read(sd, &intBuffer[i++], (sizeof intBuffer) - receivedBytes);
        receivedBytes += readBytes;
    }
    //Convert byte-array to int
    int receivedInt = *(int *) intBuffer;
    
    //Converts int to host-byte-order and returns
    return ntohl(receivedInt);
}

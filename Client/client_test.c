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

#define PORT 8189

void sendFile(int sd, char *filename);
void downloadFile(int sd, char *filename);

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
    }
    
    //Now, send filename (this should be edited to include only the filename, and not the filepath)
    char *filename_to_server = strdup(filename);
    filename_to_server[strlen(filename_to_server)] = '\n'; //'\n' necessary for readLine() to work in server
    success = write(sd, filename_to_server, sizeof(char) * strlen(filename_to_server));
    if(success<0) {
        printf("Error sending filename\n");
    }
    
    //Wait for server to send acknowledgment (doesn't work properly. The number received here is different to the number sent from the server). This step is necessary to ensure server processes filename and file separately
    int response;
    success = read(sd, &response, sizeof(int));
    if(success<0) {
        printf("Error receiving acknowledgment\n");
    }
    /*if(response != ACKNOWLEDGMENT) {
        printf("Error. Acknowledgment not received\n");
        printf("%d\n", response);
    }*/
    
    //Now, open file
    FILE *fpin = fopen(filename, "rb");
    if(fpin == NULL) {
        printf("Couldn't open file\n");
        exit(EXIT_FAILURE);
    }
    //Now, read file into array, one byte at a time
    char *file = (char *) malloc (sizeof(char));
    char *buffer = (char *) malloc(sizeof(char));
    int noBytesRead;
    noBytesRead = fread(buffer, 1, 1, fpin);
    int i=0;
    while(noBytesRead != 0) {
        file[i++] = *buffer;
        noBytesRead = fread(buffer, 1, 1, fpin);
        file = realloc(file, (i+1)*sizeof(char));
    }
    
    //Now, send array (containing file data) to server
    success = write(sd, file, sizeof(char) * i);
    if(success<0) {
        printf("Error sending file\n");
    }
    
    //Free allocated memory and close resources
    fclose(fpin);
    free(file);
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
    
    //Wait for server to send file, and read it one byte at a time
    char *file = (char *) malloc(sizeof(char));
    char *buffer = (char *) malloc(sizeof(char));
    int n; int i=0;
    n = read(sd, buffer, sizeof(char));
    while(n > 0) {
        file[i++] = *buffer;
        n = read(sd, buffer, sizeof(char));
        file = realloc(file, (i+1)*sizeof(char));
    }
    
    //When server has finished sending file, write array (containing file contents) out to file
    FILE *fpout = fopen(filename, "w");
    fwrite(file, i, sizeof(char), fpout);
    
    //Free all memory and close resources
    fclose(fpout);
    free(file);
    free(buffer);
}

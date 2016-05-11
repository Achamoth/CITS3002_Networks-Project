#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#define PORT 8189

void error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(void) {
    int port = PORT;
    struct sockaddr_in server_address;
    struct hostent *server;
    
    int sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sd < 0) {
        perror("Error: ");
        exit(EXIT_FAILURE);
    }
    
    server = gethostbyname("localhost");
    if(server == NULL) {
        printf("Error: No such host\n");
        exit(EXIT_FAILURE);
    }
    
    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
    server_address.sin_port = htons(port);
    
    if(connect(sd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        error("Error connecting");
    }
    
    /*char line[256];
    scanf("%s", line);
    
    while(!strcmp("BYE", line)) {
        int n = write(sd, line, strlen(line));
        if(n<0) {
            error("Error writing to socket");
        }
        bzero(line, 256);
        n = read(sd, line, 255);
        if(n<0) {
            error("Error reading from socket");
        }
        printf("%s\n", line);
    }*/
    shutdown(sd, SHUT_RDWR);
    close(sd);
    
          return 0;
}

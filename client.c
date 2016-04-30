//
//  Client.c
//  
//
//  Created by Ammar Abu Shamleh on 30/04/2016.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

int circleSize;
char *requiredCircleMember;
int noOfReqCircleMembers;

int main(int argc, char **argv) {
    //Initial set up stuff
    circleSize = 0; //Until user specifies otherwise
    requiredCircleMember = NULL;
    noOfReqCircleMembers = 0;
    
    //Check number of arguments
    if(argc == 1) {
        printf("Program requires at least 1 command line option\n");
        exit(EXIT_FAILURE);
    }
    
    //For processing commnad line args
    char *filenameUpload = NULL; //For uploading files to server
    char *filenameDownload = NULL; //For downloading files from server
    //char *filenameVouch = NULL; //For vouching for files
    char *certUpload = NULL; //For uploading certificates
    //char *certVouch = NULL; //For vouching for files
    char *hostName = NULL;
    int port;
    //FILE *fpupload = NULL;
    //FILE *fpfetch = NULL;
    
    bool aflag = false;
    bool cflag = false;
    bool fflag = false;
    bool hflag = false;
    bool lflag = false;
    bool nflag = false;
    bool uflag = false;
    bool vflag = false;
    
    //Parse command line args using getopt
    char c;
    while((c = getopt(argc, argv, "a:c:f:h:ln:u:v:")) != -1) {
        switch (c) {
            case 'a':
                //Upload specified file to server, replacing existing file if needed
                aflag = true;
                filenameUpload = optarg;
                break;
                
            case 'c':
                //Specify minimum required circle of trust circumference
                cflag = true;
                circleSize = atoi(optarg);
                break;
                
            case 'f':
                //Fetch existing file from server
                fflag = true;
                filenameDownload = optarg;
                break;
                
            case 'h':
                //Provide remote address hosting server
                hflag = true;
                char *copy = strdup(optarg);
                hostName = strtok(copy, ":");
                port = atoi(strtok(NULL, ":"));
                break;
                
            case 'l':
                //List all stored files on server and how they're protected
                lflag = true;
                break;
                
            case 'n':
                //Require circle of trust to involve named person
                nflag = true;
                requiredCircleMember = optarg;
                noOfReqCircleMembers++;
                break;
                
            case 'u':
                //Upload certificate to server
                uflag = true;
                certUpload = optarg;
                break;
                
            case 'v':
                vflag = true;
                certUpload = optarg;
                //NEED TO DEAL WITH SECOND OPTION ARGUMENT
                break;
        }
    }
    
    //Test command line interpretation
    if(aflag) printf("Upload file %s\n", filenameUpload);
    if(cflag) printf("Circle of size %d\n", circleSize);
    if(fflag) printf("Fetch file %s\n", filenameDownload);
    if(hflag) printf("Provide remote address of server. %s:%d\n", hostName, port);
    if(lflag) printf("List all stored files and their protection\n");
    if(nflag) printf("Require circle of trust to involve %s\n", requiredCircleMember);
    if(uflag) printf("Upload certificate %s\n", certUpload);
    if(vflag) printf("Not ready for -v yet\n");
    printf("\n");
}

/*
	CITS3002 Project 2016
	Name:			Ammar Abu Shamleh, Pradyumn Vij, Goce Krlevski 
	Student number:	21469477
	Date:			d/m/2015
*/

#include "client.h"

//  Using ':' as starting of OPTLIST silences getopt()
#define OPTLIST ":a:c:f:h:ln:u:v:"

int circleSize;
char **requiredMembers;
int minTrustedMembers;
int noOfReqCircleMembers;

void usage(char* program){
	fprintf(stderr, "\n%s Usage:\n\n"
			"  -a [filename]\t\t\tAdd or replace a file on the oldtrusty server.\n"
			"  -c [number]\t\t\tProvide the minimum circle of trust size.\n"
			"  -f [filename]\t\t\tFetch an existing file from the oldtrusty server.\n"
			"  -h [hostname:port]\t\tProvide the remote address hosting the oldtrusty server.\n"
			"  -l \t\t\t\tList all stored files and protection.\n"
			"  -n [member_name]\t\tAdd a new member to the circle of trust with \"member_name\".\n"
			"  -u [filename]\t\t\tUpload a certificate to the oldtrusty server.\n"
			"  -v [filename] [certificate]\tVouch for the authenticity of an existing file on the\n"
			"\t\t\t\toldtrusty server using the provided named certificate.\n\n", program);
}

int main(int argc, char **argv) {
    //Initial set up stuff
    circleSize = 0; //Until user specifies otherwise
    requiredMembers = NULL;
    printf("\n"); //For neat spacing
    
    
    //Check number of arguments
    if(argc < 2 || argv[1][0] != '-') {
        fprintf(stderr, "%s: requires at least 1 valid command line option.\n", argv[0]);
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    
    //  These can be defined within the switch statement once functions 
    //  are being called from within the switch statement.
    char* uploadFilename;
    char* downloadFilename;
    char* hostName;
    char* newMember;
    char* uploadCertificate;
    char* checkFilenameAuth;
    char* authorisingCert;
    int port = 0;
    bool success = false;
    
    //Parse command line args using getopt
    int option;
    while((option = getopt(argc, argv, OPTLIST)) != -1) {
        switch(option){
        	case 'a':
                //  Upload specified file to server, replacing file if needed
                uploadFilename = strdup(optarg);
                success = uploadFile(uploadFilename, false);
                if(!success) {
                    //DO SOMETHING
                }
                break;
            case 'c':
                //  Specify minimum required circle of trust circumference
                circleSize = atoi(optarg);
                break;
            case 'f':
                //  Fetch existing file from server
                downloadFilename = strdup(optarg);
                success = downloadFile(downloadFilename);
                if(!success) {
                    //DO SOMETHING
                }
                break;
            case 'h':
                //  Provide remote address hosting server
                hostName = strdup(optarg); // overprovisions memory
                sscanf(optarg, "%[^:]:%d", hostName, &port);
                getAddress(hostName, port);
                break;
            case 'l':
                //  List all stored files on server and how they're protected
                listFiles();
                break;
            case 'n':
                //  Require circle of trust to involve named person
                newMember = strdup(optarg);
                noOfReqCircleMembers++;
                newRequiredMember(newMember);
                break;
            case 'u':
                //  Upload certificate to server
                uploadCertificate = strdup(optarg);
                uploadFile(uploadCertificate, true);
                break;	
            case 'v':
            	// Vouch for authenticity of filename using named certificate
            	checkFilenameAuth = strdup(optarg);
            	if(optind < argc &&  *argv[optind] != '-'){
            		authorisingCert = strdup(argv[optind]);
            		optind++;
            	}
            	else{
            		fprintf(stderr, "%s: '-%c' requires TWO arguments, for example\n"
            			 "-%c [filename on server] [certificate to vouch with]\n", 
            			 	argv[0], optopt, optopt);
            	}
                //FUNCTION CALL
                break;
            case ':':
            	//  valid option, missing argument
            	fprintf(stderr, "%s: '-%c' option requires an argument.\n", argv[0], optopt);
            	return EXIT_FAILURE;
            case '?':
            	//  invalid option
            	fprintf(stderr, "%s: Unknown option '-%c' is invalid.\n", argv[0], optopt);
            	return EXIT_FAILURE;
        }
    }
    
    printf("Required circle size: %d\n", circleSize);
    printf("Required circle members:\n");
    for(int i=0; i<noOfReqCircleMembers; i++) printf("%s\n", requiredMembers[i]);
    freeCircleMembers();
    printf("\n");
    
    return EXIT_SUCCESS;
}

//Maintains list of required circle members
void newRequiredMember (char *newMember) {
    requiredMembers = realloc(requiredMembers, noOfReqCircleMembers * sizeof(char *));
    requiredMembers[noOfReqCircleMembers-1] = malloc(strlen(newMember));
    requiredMembers[noOfReqCircleMembers-1] = strdup(newMember);
}

//Frees memory allocated for storing required circle members
void freeCircleMembers () {
    for(int i=0; i < noOfReqCircleMembers; i++) {
        free(requiredMembers[i]);
    }
    free(requiredMembers);
}

//Uploads file to server. Param isCert identifies whether file being uploaded is a certificate
bool uploadFile (char *filename, bool isCert) {
    //TODO
    if(!isCert)printf("Upload file \"%s\"\n", filename);
    else printf("Upload certificate \"%s\"\n", filename);
    return false;
}

//Downloads file from server
bool downloadFile (char *filename) {
    //TODO
    printf("Download file\"%s\"\n", filename);
    return false;
}

//Get and report address of remote server
void getAddress (char *hostName, int port) {
    //TODO
    printf("Get address of \"%s:%d\"\n", hostName, port);
}

//List all files on server along with their protection
void listFiles () {
    //TODO
    printf("List all files and their protection\n");
}

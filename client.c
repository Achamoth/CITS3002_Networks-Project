//
//  Client.c
//  
#include "client.h"

//  Using ':' as starting of OPTLIST silences getopt()
#define OPTLIST ":a:c:f:h:ln:u:v:"

int circleSize;
char *requiredCircleMember;
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
    requiredCircleMember = NULL;
    
    //Check number of arguments
    if(argc < 2) {
        fprintf(stderr, "%s: requires at least 1 command line option.\n", argv[0]);
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

    // bools for testing.
    bool aflag = false;
    bool cflag = false;
    bool fflag = false;
    bool hflag = false;
    bool lflag = false;
    bool nflag = false;
    bool uflag = false;
    bool vflag = false;
    
    //Parse command line args using getopt
    int option;
    while((option = getopt(argc, argv, OPTLIST)) != -1) {
        switch(option){
        	case 'a':
                //  Upload specified file to server, replac file if needed
                uploadFilename = strdup(optarg);
                aflag = !aflag;
                break;
            case 'c':
                //  Specify minimum required circle of trust circumference
                circleSize = atoi(optarg);
                cflag = !cflag;
                break;
            case 'f':
                //  Fetch existing file from server
                downloadFilename = strdup(optarg);
                fflag = !fflag;
                break;
            case 'h':
                //  Provide remote address hosting server
                hostName = strdup(optarg); // overprovisions memory
                sscanf(optarg, "%[^:]:%d", hostName, &port);
                hflag = !hflag;
                break;
            case 'l':
                //  List all stored files on server and how they're protected
                lflag = !lflag;
                break;
            case 'n':
                //  Require circle of trust to involve named person
                newMember = strdup(optarg);
                noOfReqCircleMembers++;
                nflag = !nflag;
                break;
            case 'u':
                //  Upload certificate to server
                uploadCertificate = strdup(optarg);
                uflag = !uflag;
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
                vflag = !vflag;
                break;
            case ':':
            	//  valid option, missing argument
            	printf("kek");
            	fprintf(stderr, "%s: '-%c' option requires an argument.\n", argv[0], optopt);
            	return EXIT_FAILURE;
            case '?':
            	//  invalid option
            	fprintf(stderr, "%s: Unknown option '-%c' is invalid.\n", argv[0], optopt);
            	return EXIT_FAILURE;
            default:
            	//  Improper use of client program.
            	usage(argv[0]);
        }
    }
    
    //Test command line interpretation
    if(aflag) printf("Upload file %s\n", uploadFilename);
    if(cflag) printf("Circle of size %d\n", circleSize);
    if(fflag) printf("Fetch file %s\n", downloadFilename);
    if(hflag) printf("Provide remote address of server %s with port %d\n", hostName, port);
    if(lflag) printf("List all stored files and their protection\n");
    if(nflag) printf("Require circle of trust to involve %s\n", newMember);
    if(uflag) printf("Upload certificate %s\n", uploadCertificate);
    if(vflag) printf("Filename: %s, authorised by certificate: %s\n", checkFilenameAuth, authorisingCert);
    printf("\n");

    return EXIT_SUCCESS;
}

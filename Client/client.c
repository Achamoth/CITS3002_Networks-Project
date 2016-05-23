/*
	CITS3002 Project 2016
	Name:			Ammar Abu Shamleh, Pradyumn Vij 
	Student number: 21521274, 21469477
    Date:           May 2016
*/

#include "client.h"
// Using ':' as starting of OPTLIST silences getopt()
#define OPTLIST ":a:c:f:h:ln:u:v:"
// Program name
const char* programName;


/*
	Prints usage instructions for client application and exits
*/
void usage(void){
	fprintf(stderr, "\n%s Usage:\n\n"
		"  -a [filename]\t\t\tAdd or replace a file on the oldtrusty server.\n"
		"  -c [number]\t\t\tProvide the minimum circle of trust length.\n"
		"  -f [filename]\t\t\tFetch an existing file from the oldtrusty server.\n"
		"  -h [hostname:port]\t\tProvide the remote address hosting the oldtrusty server.\n"
		"  -l \t\t\t\tList all stored files and how they are protected.\n"
		"  -n [member_name]\t\tSpecify a member you wish to be in the circle of trust with \"member_name\".\n"
		"  -u [filename]\t\t\tUpload a certificate to the oldtrusty server.\n"
		"  -v [filename] [certificate]\tVouch for the authenticity of an existing file on the\n"
		"\t\t\t\toldtrusty server using the provided named certificate.\n\n", programName);
	exit(EXIT_FAILURE);
}



/*
	Main

	Command Line Parser
	
    @param argc 	Number of arguments (int)
	@param argv		Arguments (array of char arrays)
*/
int main(int argc, char **argv) {
    // Program name without /, cast to constant for file
    programName = (const char*) strrchr(argv[0], '/') + 1;
    
	//Check number of arguments
    if(argc < 2 || argv[1][0] != '-') {
        fprintf(stderr, "%s: requires at least 1 valid command line option.\n",
        	programName);
        usage();
    }
    
    char* host = NULL;
	char* port = NULL;
	char* fileName = NULL;
	char* certificateName = NULL;
	char* memberName = NULL;
	int minCircleSize = 0;
	actionType action = NONE;
    bool requiredMember = false;

    
    //	Parse command line args
    int option;
    while((option = getopt(argc, argv, OPTLIST)) != -1) {
        switch(option){
        	case 'a':
                //  Add specified file to server, replacing file if needed
        		action = PUSH;
                fileName = strdup(optarg);
                break;
            case 'c':
                //  Specify minimum required circle of trust
                minCircleSize = atoi(optarg);
                break;
            case 'f':
                //  Fetch existing file from server
            	action = PULL;
                fileName = strdup(optarg);
                break;
            case 'h':
                //  Following assumes IPV 4, can change to include IPV6
                //  Just have to ask user for format in [host]:port form 
                //  Provide remote address hosting server
                host = strdup(optarg); // overprovisions memory
                //  Largest port length is 5 characters
                port = (char*) malloc(sizeof(char)*6);
                sscanf(optarg, "%[^:]:%s", host, port);
                break;
            case 'l':
                //  List all stored files on server and how they're protected
            	action = LIST;
                break;
            case 'n':
                //  Require circle of trust to involve named person
                requiredMember = true;
            	memberName = strdup(optarg);
                break;
            case 'u':
                //  Upload new certificate to server
            	action = PUSH_CERT;
                certificateName = strdup(optarg);
                break;	
            case 'v':
            	// Vouch for authenticity of filename using named certificate
            	action = VOUCH;
            	fileName = strdup(optarg);
            	if(optind < argc &&  *argv[optind] != '-'){
            		certificateName = strdup(argv[optind]);
            		optind++;
            	}
            	else{
            		fprintf(stderr, "%s: '-%c' option requires TWO arguments\n", 
            			programName, optopt);
            		usage();
            	}
                break;
            case ':':
            	//  valid option, missing argument
            	fprintf(stderr, "%s: '-%c' option requires an argument.\n", 
                    programName, optopt);
            	usage();
            case '?':
            	//  invalid option
            	fprintf(stderr, "%s: Unknown option '-%c'.\n", programName, 
                    optopt);
            	usage();
        }
    }

    //  Run user request
    parseRequest(host, port, action, fileName, certificateName, minCircleSize, 
        memberName, requiredMember);

    //  Free allocated memory on heap
    free(host);
    free(port);
    if(fileName != NULL) free(fileName);
    if(certificateName != NULL) free(certificateName);
    if(memberName != NULL) free(memberName);
    
    return EXIT_SUCCESS;
}

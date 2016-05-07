/*
	CITS3002 Project 2016
	Name:			Ammar Abu Shamleh, Pradyumn Vij, Goce Krlevski 
	Student number:	21469477
	Date:			d/m/2015
*/

#include "client.h"
// Using ':' as starting of OPTLIST silences getopt()
#define OPTLIST ":a:c:f:h:ln:u:v:"
// Program name
static const char* programName;


int circleSize;
char **requiredMembers;
int minTrustedMembers;
int noOfReqCircleMembers;





/*
	Prints usage instructions for client application and exits
*/
static void usage(void){
	fprintf(stderr, "\n%s Usage:\n\n"
			"  -a [filename]\t\t\tAdd or replace a file on the oldtrusty server.\n"
			"  -c [number]\t\t\tProvide the minimum circle of trust length.\n"
			"  -f [filename]\t\t\tFetch an existing file from the oldtrusty server.\n"
			"  -h [hostname:port]\t\tProvide the remote address hosting the oldtrusty server.\n"
			"  -l \t\t\t\tList all stored files and how they are protected.\n"
			"  -n [member_name]\t\tAdd a new member to the circle of trust with \"member_name\".\n"
			"  -u [filename]\t\t\tUpload a certificate to the oldtrusty server.\n"
			"  -v [filename] [certificate]\tVouch for the authenticity of an existing file on the\n"
			"\t\t\t\toldtrusty server using the provided named certificate.\n\n", programName);
	exit(EXIT_FAILURE);
}

/*
	Open a socket and connect using TCP protocol
	@param host 	Server address
	@param port 	Server port
	@return 		Open socket
*/
static int openConnection(const char *host, const char *port){
	addrinfo hostReq;
	addrinfo *hostFound;
	int addrInfo_Error;
	int socketDescriptor;

	//  Make sure struct is initialised cleanly
	memset(&hostReq, 0, sizeof(hostReq));
	//  Set the host requirements addrinfo
	hostReq.ai_family = AF_UNSPEC;
    //  TCP stream (SSL requires) reliability
	hostReq.ai_socktype = SOCK_STREAM;
	hostReq.ai_protocol = IPPROTO_TCP;

	// Error check on getaddrinfo
	if((addrInfo_Error = getaddrinfo(host, port, &hostReq, &hostFound)) != 0){
		fprintf(stderr, "%s: Get Address Error: %s\n", programName, 
			gai_strerror(addrInfo_Error));
		usage();
	}
	// IPV6 Scoped ID check
	/*if(hostFound->ai_family == PF_INET6){
		// If sin6_scope_id of ipv6 type address is unchanged ask for scoped address
		sockaddr_in
		if(hostFound->ai_addr->sin6_scope_id == 0){
			fprintf(stderr, "%s: IPv6 Unscoped Error: When using IPv6 addresses, "
				"make sure they are entered in a scoped format.\n", programName);
		}
		usage();
	}*/

	// Create a socket descriptor
	socketDescriptor = socket(hostFound->ai_family,
								hostFound->ai_socktype,
									hostFound->ai_protocol);
	if(socketDescriptor < 0){
		perror("Socket Error");
		exit(EXIT_FAILURE);
	}

	// Connect to socket
	if(connect(socketDescriptor, hostFound->ai_addr, hostFound->ai_addrlen) < 0){
		close(socketDescriptor);
		perror("Connect Error");
		exit(EXIT_FAILURE);
	}
	// Free the addrinfo struct
	freeaddrinfo(hostFound);

	// Return socket descript to close after operation 
	return socketDescriptor;
}

int main(int argc, char **argv) {
    // Initial set up stuff
    circleSize = 0; //Until user specifies otherwise
    requiredMembers = NULL;
    printf("\n"); //For neat spacing
    // program name without /, cast to constant for file
    programName = (const char*) strrchr(argv[0], '/');
    
    
    //Check number of arguments
    if(argc < 2 || argv[1][0] != '-') {
        fprintf(stderr, "%s: requires at least 1 valid command line option.\n",
        	programName);
        usage();
    }
    
    //  These can be defined within the switch statement once functions 
    //  are being called from within the switch statement.
    char* hostArg;
    char* port;
    char* uploadFilename;
    char* downloadFilename;
    char* newMember;
    char* uploadCertificate;
    char* checkFilenameAuth;
    char* authorisingCert;
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
                hostArg = strdup(optarg); // overprovisions memory
                // largest port length is 5 characters
                port = (char*) malloc(sizeof(char)*6);
                sscanf(optarg, "%[^:]:%s", hostArg, port);
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
                    vouchForFile(checkFilenameAuth, authorisingCert);
            	}
            	else{
            		fprintf(stderr, "%s: '-%c' option requires TWO arguments\n", 
            			programName, optopt);
            		usage();
            	}
                break;
            case ':':
            	//  valid option, missing argument
            	fprintf(stderr, "%s: '-%c' option requires an argument.\n", programName, optopt);
            	usage();
            case '?':
            	//  invalid option
            	fprintf(stderr, "%s: Unknown option '-%c'.\n", programName, optopt);
            	usage();
        }
    }
    
    printf("Required circle size: %d\n", circleSize);
    printf("Required circle members:\n");
    for(int i=0; i<noOfReqCircleMembers; i++) printf("%s\n", requiredMembers[i]);
    freeCircleMembers();
    printf("\n");

    int sd = openConnection(hostArg, port);

    close(sd); 

    
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
void getAddress (char *hostAddress, int port) {
    //TODO
    printf("Get address of \"%s:%d\"\n", hostAddress, port);
}

//List all files on server along with their protection
void listFiles () {
    //TODO
    printf("List all files and their protection\n");
}

//Vouch for specified file (param 1) with designated certificate (param 2)
void vouchForFile(char *filename, char *certname) {
    //TODO
    printf("Vouch for file \"%s\" with certificate \"%s\"\n", filename, certname);
}

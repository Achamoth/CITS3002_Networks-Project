/*
    CITS3002 Project 2016
    Name:           Ammar Abu Shamleh, Pradyumn Vij, Goce Krlevski 
    Student number: 21469477
    Date:           d/m/2015
*/
#include "client.h"
#define PUBLIC_KEY "certificates/public.crt"
#define PRIVATE_KEY "certificates/private.key"

/*
    openTCPConnection
    Open a socket and connect using TCP protocol
    @param host     Server address
    @param port     Server port
    @return         Open socket
*/
int openTCPConnection(const char *host, const char *port){
    addrinfo hostReq;
    addrinfo *hostFound;
    int addrInfo_Error;
    int socketDescriptor;

    //  Make sure struct is initialised cleanly
    memset(&hostReq, 0, sizeof(hostReq));
    //  Set the host requirements addrinfo,
    //  Can be changed to flex between both standards using AF_UNSPEC
    hostReq.ai_family = AF_INET;
    //  TCP stream (SSL requires) reliability
    hostReq.ai_socktype = SOCK_STREAM;
    hostReq.ai_protocol = IPPROTO_TCP;

    // Error check on getaddrinfo
    if((addrInfo_Error = getaddrinfo(host, port, &hostReq, &hostFound)) != 0){
        fprintf(stderr, "%s: Get Address Error: %s\n", programName, 
            gai_strerror(addrInfo_Error));
        usage();
    }
    else{
        fprintf(stdout, "%s: Server Found.\n", programName);
    }

    // Create a socket descriptor
    socketDescriptor = socket(hostFound->ai_family,
                                hostFound->ai_socktype,
                                    hostFound->ai_protocol);
    if(socketDescriptor < 0){
        perror("Socket Error");
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: Establishing TCP connection...\n", programName);
    }

    // Connect to socket
    if(connect(socketDescriptor, hostFound->ai_addr, hostFound->ai_addrlen) < 0){
        close(socketDescriptor);
        perror("Connect Error");
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: TCP connection established.\n\n", programName);
    }
    // Free the addrinfo struct
    freeaddrinfo(hostFound);

    // Return socket descript to close after operation 
    return socketDescriptor;
}


/*
    secureConnection
    Intialise SSL context and negotiate secure transmission with
    server
*/
void secureConnection(const char* host, const char* port){
    //  Initialise SSL library--------------------------------------------------
    SSL_load_error_strings();
    //  OpenSSL 1.02 and below load crypt:
    SSL_library_init(); //  Loads encryption and hashing for SSL program
    OpenSSL_add_all_algorithms(); // cryptogrophy, intialise ciphers and digests
    //  Create SSL context - client in this case using SSLv3
    SSL_CTX *sslContext = SSL_CTX_new(SSLv3_client_method());
    if(sslContext == NULL){
        fprintf(stderr, "\n%s: SSL: Failed to create new context.\n", programName);
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "\n%s: SSL context created.\n", programName);
    }

    //  Private Key
    //  Load private key corresponding to client certificate to allow for 
    //  SSL handshake to decrypt any communication encoded by the public key
    if(SSL_CTX_use_PrivateKey_file(sslContext, PRIVATE_KEY, 
                                                        SSL_FILETYPE_PEM) != 1){
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: Client Private Key found.\n", programName);
    }

    //  Load Public Certificate including public key into ssl session-----------
    //  Load the client certificate in PEM format
    if(SSL_CTX_use_certificate_file(sslContext, PUBLIC_KEY, 
                                                    SSL_FILETYPE_PEM) != 1){
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: Client Public Key found.\n", programName);
    }

    //  Check public - private key pair integrity
    if(!SSL_CTX_check_private_key(sslContext)){
        fprintf(stderr, "%s: SSL: Client Private key does not match client" 
            "certificate public key.\n", programName);
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: Public - Private key relationship verified.\n", 
            programName);
    }
    
    fprintf(stdout, "%s: Contacting server...\n", programName);
    //  Find and open TCP socket to server
    int serverSocket = openTCPConnection(host, port);

    // Create SSL structure
    SSL *ssl = SSL_new(sslContext);
    if(ssl == NULL){
        fprintf(stderr, "%s: SSL: Failed to create new session.\n", programName);
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: SSL session created.\n", programName);
    }

    // Set socket descript to SSL context
    SSL_set_fd(ssl, serverSocket);

    // Initiate SSL handshake
    if(SSL_connect(ssl) != 1){
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: SSL handshake successful.\n", programName);
    }

    fprintf(stdout, "%s: SSL session encrypted using:\n\t%s\n", programName,
        SSL_get_cipher(ssl));

    X509* serverCertificate = SSL_get_peer_certificate(ssl);
    if(serverCertificate == NULL){
        fprintf(stderr, "%s: No public certificate provided by server, "
            "disconnecting.", programName);
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "%s: Server certificate received.\n", programName);
    }

    fprintf(stdout, "Server certificate details:\n");
    //  Print Issuer to stdout
    X509_NAME_print_ex_fp(stdout, X509_get_issuer_name(serverCertificate), 2, 
        XN_FLAG_MULTILINE);
    fprintf(stdout, "\n%s: Server certificate subject (if any):\n", programName);
    X509_NAME_print_ex_fp(stdout, X509_get_subject_name(serverCertificate), 2, 
        XN_FLAG_MULTILINE);

    //TODO ADD DATA TO BE EXCHANGED

    char buf[1024];
    int bytes;
    char *message = "hello world!";

    SSL_write(ssl, message, strlen(message));
    bytes = SSL_read(ssl, buf, sizeof(buf));
    buf[bytes] = '\0';
    printf("got %d chars: '%s'\n", bytes, buf);
    free(message);

    //  Free server's certificate
    X509_free(serverCertificate);
    // Notify SSL session to shutdown
    fprintf(stdout, "%s: Closing SSL session.\n", programName);
    SSL_shutdown(ssl);
    fprintf(stdout, "Closing TCP connection.\n");
    // Notify that socket wants to close
    shutdown(serverSocket, SHUT_RDWR);   // Graceful TCP shutdown
    close(serverSocket);        // close (destroy) TCP socket
    SSL_free(ssl);              // free ssl structure
    SSL_CTX_free(sslContext);   // free ssl context
}
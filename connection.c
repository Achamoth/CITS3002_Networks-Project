/*
    CITS3002 Project 2016
    Name:           Ammar Abu Shamleh, Pradyumn Vij, Goce Krlevski 
    Student number: 21469477
    Date:           d/m/2015
*/
#include "client.h"

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


/*
    secureConnection
    Intialise SSL context and negotiate secure transmission with
    server
*/
void secureConnection(const char* host, const char* port){
    //  Initialise SSL library
    SSL_load_error_strings();
    SSL_library_init(); //  Loacs encryption and hashing for SSL program
    OpenSSL_add_all_algorithms(); // cryptogrophy
    //  Create SSL context - client in this case using SSLv3
    SSL_CTX *sslContext = SSL_CTX_new(SSLv3_client_method());
    if(sslContext == NULL){
        fprintf(stderr, "%s: SSL: Failed to create new context.\n", programName);
        exit(EXIT_FAILURE);
    }

    //  Public Key
    //  Load the client certificate (need to have position in project folder for
    //  certs)
    if(SSL_CTX_use_certificate_file(sslContext, RSA_CLIENT_CERT, 
                                                    SSL_FILETYPE_PEM) < 1){
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Be a verified Client if asked-------------------------------------------
    // Load client certificate into context
    if(SSL_CTX_use_certificate_file(sslContext, RSA_CLIENT_CERT), 
        SSL_FILETYPE_PEM) < 1){
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    //  Private Key
    //  Load private key corresponding to client certificate
    if(SSL_CTX_use_PrivateKey_file(sslContext, RSA_CLIENT_KEY, 
                                                    SSL_FILETYPE_PEM) < 1){
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    //  Check if certificate and key pair provided match
    if(!SSL_CTX_check_private_key(sslContext)){
        fprintf(stderr, "%s: SSL: Client Private key does not match client" 
            "certificate public key.\n", programName);
        exit(EXIT_FAILURE);
    }

    //  Do not have CA in this case so don't have CA file
    //  Set flag in context to request server's certificate verification
    //  Forces server to have certificate, if SSL or TLS will have one anyway
    SSL_CTX_set_verify(sslContext, SSL_VERIFY_PEER, NULL);
    // Below is a chain of trust function but is similar to the ring of trust.
    // SSL_CTX_set_verify_depth(sslContext,1);
    // -------------------------------------------------------------------------
    
    //  Connect to server via TCP
    int serverSocket = openTCPConnection(host, port);

    // Create SSL structure
    SSL *ssl = SSL_new(sslContext);
    if(ssl == NULL){
        fprintf(stderr, "%s: SSL: Failed to create new context.\n", programName);
        exit(EXIT_FAILURE);
    }

    // Connect SSL structure with the server TCP socket descriptor
    SSL_set_fd(ssl, serverSocket);
    // Initiate SSL handshake
    if(SSL_connect(ssl, serverSocket) < 0){
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "SSL connection encrypted using %s\n", SSL_get_cipher(ssl));

    X509* serverCertificate = SSL_get_peer_certificate(ssl);
    if(serverCertificate == NULL){
        fprintf(stderr, "%s: No SSL certificate provided by server, disconnecting.", 
            programName);
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stdout, "SSL certificate received.\n");
    }
    fprintf(stdout, "Server certificate:\nIssuer:\n");

    //  Print Issuer to stdout
    X509_Name_print_ex_fp(stdout, X509_get_issuer_name(serverCertificate), 2, 
        XN_FLAG_MULTILINE);
    fprintf(stdout,"Subject:\n");
    X509_Name_print_ex_fp(stdout, X509_get_subject_name(serverCertificate), 2, 
        XN_FLAG_MULTILINE);

    //TODO ADD DATA TO BE EXCHANGED

    SSL_write(ssl, "hello world!", strlen("hello World!") + 1);
    SSL_read(ssl, buf, sizeof(buf) -1);
    buf[err] = '\0';
    printf("got %d chars: '%s'\n", err, buf);


    fprintf(stdout, "Closing SSL connection.\n");
    //  Free server's certificate
    X509_free(serverCertificate);
    // Notify SSL session to shutdown
    SSL_shutdown(ssl);
    close(serverSocket);        // close TCP socket
    SSL_free(ssl);              // free ssl structure
    SSL_CTX_free(sslContext);   // free ssl context
}
/*
    CITS3002 Project 2016
    Name:           Ammar Abu Shamleh, Pradyumn Vij, Goce Krlevski 
    Student number: 21469477
    Date:           d/m/2015
*/
#include "client.h"

/*
    sendMessage
    Parse the type of message required, direct user input appropriately
    @param host         Server address (IPV4, can be changed to IPV6 in main in the future)
    @param port         Server port
    @param action       Purpose of message
    @param file         File to upload / download / vouch
    @param certificate  Certificate to upload / sign
    @param minCircle    Minimum circle of trust required
    @param member       Member's name should be in circle of trust
    @return void        Maybe change to success / fail
*/
void parseRequest(char *host, char *port, actionType action, char *file, char *certificate, 
                    int minCircle, char *member){
    secureConnection(host, port);
}

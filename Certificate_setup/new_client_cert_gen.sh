#!/bin/sh


#
#  Created by George Krlevski on 11/05/2016.
#


hr="- - - - - - - - - - - - - - - - - - - - - - -"
br=""
strength=2048
validFor=365

infoMessage="Usage:  sh new_client_cert_gen.sh [client_name@domain.com]"

if [ $# -ne 1 ];
then
echo $infoMessage
exit 2
fi

if [ $1 = "--help" ];
then
echo $infoMessage
exit 2
fi

if [ ! -d ./client/ ];
then
echo "Creating Client folder..."
mkdir ./client/
mkdir ./client/keys/
mkdir ./client/requests/
mkdir ./client/certificates/

fi

export OPENSSL_CONF=./configurations/client_configuration.cnf

client=$1
clientK=./client/keys/$client.key
clientR=./client/requests/$client.csr
clientC=./client/certificates/$client.crt


echo $br
echo $hr
echo "CREATING CLIENT KEY"
echo $hr

openssl genrsa -des3 -out $clientK $strength

echo $br
echo $hr
echo "CREATING CLIENT CERTIFICATE REQUEST"
echo $hr

openssl req -new -key $clientK -out $clientR

echo $br
echo $hr
echo "CA SIGNING AND ISSUING CLIENT CERTIFICATE"
echo $hr

openssl x509 -req -in $clientR -out $clientC -CA ./ca/ca.crt -CAkey ./ca/ca.key -CAcreateserial -days $validFor

echo $br
echo $hr
echo "GENERATING CERTIFICATE ON CONSOLE"
echo $hr

openssl x509 -in $clientC -text -noout
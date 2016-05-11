#!/bin/sh


#
#  Created by George Krlevski on 11/05/2016.
#
hr="- - - - - - - - - - - - - - - - - - - - - - -"
br=""
strength=2048
validFor=365

infoMessage="Usage:  sh server_cert_gen.sh [server_name@domain.com]"

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

if [ ! -d ./server/ ];
then
echo "Creating Server folder..."
mkdir ./server/
mkdir ./server/keys/
mkdir ./server/certificates/
mkdir ./server/requests/
fi

export OPENSSL_CONF=./configurations/server_configuration.cnf

server=$1
serverK=./server/keys/$server.key
serverR=./server/requests/$server.csr
serverC=./server/certificates/$server.crt

echo $br
echo $hr
echo "CREATING SERVER KEY"
echo $hr

openssl genrsa -des3 -out $serverK $strength

echo $br
echo $hr
echo "CREATING SERVER CERTIFICATE REQUEST"
echo $hr

openssl req -new -key $serverK -out $serverR

echo $br
echo $hr
echo "CA SIGNING AND ISSUING SERVER CERTIFICATE"
echo $hr

openssl x509 -req -in $serverR -out $serverC -CA ./ca/ca.crt -CAkey ./ca/ca.key -CAcreateserial -days $validFor

echo $br
echo $hr
echo "GENERATING CERTIFICATE ON CONSOLE"
echo $hr

openssl x509 -in $serverC -text -noout
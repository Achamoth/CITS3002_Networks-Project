#!/bin/sh

#
#  Created by George Krlevski on 11/05/2016.
#


hr="- - - - - - - - - - - - - - - - - - - - - - -"
br=""
strength=2048
valid=365

mkdir ca
mkdir server
mkdir server/certificates
mkdir server/keys
mkdir server/requests

mkdir client
mkdir client/certificates
mkdir client/keys
mkdir client/requests


echo '100001' > serial

touch index.txt

export OPENSSL_CONF=./configurations/CA_configuration.cnf

echo $br
echo $hr
echo "GENERATING CERTIFICATE AUTHORITY KEY"
echo $hr

openssl genrsa -des3 -out ./ca/ca.key $strength

echo $br
echo $hr
echo "GENERATING CERTIFICATE REQUEST FOR CA"
echo $hr

openssl req -new -key ./ca/ca.key -out ./ca/ca.csr

echo $br
echo $hr
echo "CA SELF-SIGNING AND ISSUING CERTIFICATE"
echo $hr

openssl x509 -req -days $valid -in ./ca/ca.csr -out ./ca/ca.crt -signkey ./ca/ca.key

echo $br
echo $hr
echo "GENERATING CERTIFICATE ON DEVICE"
echo $hr

openssl x509 -in ./ca/ca.crt -text
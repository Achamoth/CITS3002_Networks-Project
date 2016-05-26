#	Shell script to generate SSL public and private key pair
#	by: Pradyumn Vij

#	Generate a private, public key pair in a PEM file
#
#	Non encrypted so that a password is not required (but can be)
#	This should not leave the client's computer or this folder
#	Default exponent used = 65537, default numbits 1024
#	PEM file format
#	Increase bits with -pkeyopt rsa_keygen_bits:2048
#	Add pass with -aes-256-cbc -pass:password
openssl genpkey -algorithm RSA -outform PEM -out keys/private.key

#	Generate a public key
#	Generally don't need as embedded in certificate as well
#
#	Default input file form will be PEM
#	Not encrypted, otherwise would have to share password with
#	another entity prior.
#	Output file form is PEM
#	Input file is the private key
#	Output file form is public with pubout
#	Can make it more verbose by removing -text and -modulus
openssl rsa -in keys/private.key -outform PEM -out keys/public.key -pubout

#	Generate a certificate
#
#	Default input file form is PEM
#	Gather details for a certificate
#	Make the certificate a root certificate, we pretend to be a CA and
#	approve the details to be correct (in the absecne of a CA)
#	Reads the private key, generates an X509 PKCS#10 certifcate
#	Valid for 90 days
#	Req can also be used to write a private and public key as well as a cert
openssl req -new -x509 -key keys/private.key -outform PEM -out certificates/public.crt -days 90

#	Check consistency of private key
#	If all data needs to be printed add -text
openssl rsa -in keys/private.key -check -noout

#	Display certificate
openssl x509 -in certificates/public.crt -text -noout
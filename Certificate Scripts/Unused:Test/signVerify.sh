#	Shell script to test signing and verifying SSL certificates
#	by: Pradyumn Vij

#	Digitally sign the digest using the private key
#
#	Output to file hello.rsa
#	Input file on the end hello.txt
#	Using sha256 but can use any other cipher
openssl sha256 -sign private.key -out hello.rsa hello.txt

#	Verify Digital signature using public key
#
#	Using sha256, verify the signature using public key  on file
#	hello.rsa, comparing it with the original file
openssl sha256 -verify public.key -signature hello.rsa hello.txt

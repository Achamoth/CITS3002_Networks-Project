#	Shell script to sign certificate request
#	by: Ammar Abu Shamleh

#	Signs “request.csr” in “requests” folder
#	Uses “private.key” in “keys” folder for signing
#	Uses “public.crt” in “certificates” folder as certificate of signer
#	Generates “signed.crt” in “certificates” as signed certificate

#	Variable declarations
set REQUEST = “”
set SIGNER_KEY = “”
set SIGNER_CERT = “”
set SIGNED_CERT = “”

#	Ask user to input filename of request
echo “Enter the name of the request to be signed : “
read REQUEST

#	Ask user to input filename of signer key
echo “Enter the name of the signer’s key : “
read SIGNER_KEY

#	Ask user to input filename of signer certificate
echo “Enter the name of the signer’s certificate : “
read SIGNER_CERT

#	Ask user to input desired filename of signed certificate
echo “Enter the output name of the signed certificate : “
read SIGNED_CERT

#	Call openssl to sign key with specified variables
openssl x509 -req -in $REQUEST -CAkey $SIGNER_KEY -CA $SIGNER_CERT -out $SIGNED_CERT -CAcreateserial

#	Clean up
rm $REQUEST
rm certificates/signed/*.srl
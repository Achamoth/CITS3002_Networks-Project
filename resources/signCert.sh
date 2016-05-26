#	Shell script to sign certificate request
#	by: Ammar Abu Shamleh

#	Signs “request.csr” in “requests” folder
#	Uses “private.key” in “keys” folder for signing
#	Uses “public.crt” in “certificates” folder as certificate of signer
#	Generates “signed.crt” in “certificates” as signed certificate

openssl x509 -req -in requests/request.csr -CAkey keys/private.key -CA certificates/public.crt -out certificates/signed.crt -CAcreateserial

rm certificates/public.srl
rm requests/request.csr
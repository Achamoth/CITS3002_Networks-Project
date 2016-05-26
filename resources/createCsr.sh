#	Shell script to generate SSL certificate signing request using private key
#	by: Ammar Abu Shamleh

openssl req -new -sha256 -key keys/private.key -out requests/request.csr

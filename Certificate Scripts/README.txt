#	Basic instructions for certificate generation and signing
#	by: Ammar Abu Shamleh

This folder deals with creation of keys and certificates

Creating a key and certificate:
1. Run the shell script ‘createKey.sh’
2. Fill appropriate fields (i.e. directing it to where you want the key and cert saved)

Creating a certificate signing request:
1. Run the shell script ‘createReq.sh’ which creates a certificate signing request
2. It will ask for the location (i.e. a relative pathname) to the certificate you want
signed
3. It will then ask for a location to save certificate signing request

Signing a certificate with a private key:
1. Run the shell script ‘signCert.sh’
2. Enter appropriate fields (i.e. signing request to act on, private key to use, and where
to save relevant files)


EXAMPLE (Setting up a certificate for A and B, with A signing B, and B signing A):

#	Generating A’s public-private key pair (self-signed cert)
sh createKey.sh
“Enter the desired private key pathname: ” keys/A.key
“Enter the desired certificate pathname: “ certificates/A.crt

#	Generating B’s public-private key pair (self-signed cert)
sh createKet.sh
“Enter the desired private key pathname: “ keys/B.key
“Enter the desired certificate pathname: “ certificates/B.crt

#	Generating A’s certificate signing request
sh createReq.sh
“Enter the filename of the private key: “ keys/A.key
“Enter the output name of the request file: “ requests/A.csr

#	Generating B’s certificate signing request
sh createReq.sh
“Enter the filename of the private key: “ keys/B.key
“Enter the output name of the request file: “ requests/B.csr

#	Signing A’s cert with B’s key
sh signCert.sh
“Enter the name of the request to be signed : “ requests/A.csr
“Enter the name of the signer’s key : “ keys/B.key
“Enter the name of the signer’s certificate : “ certificates/B.crt
“Enter the output name of the signed certificate : “ certificates/signed/A_signed.crt

#	Signing B’s cert with A’s key
sh signCert.sh
“Enter the name of the request to be signed : “ requests/B.csr
“Enter the name of the signer’s key : “ keys/A.key
“Enter the name of the signer’s certificate : “ certificates/signed/A.crt
“Enter the output name of the signed certificate : “ certificates/signed/B_signed.crt

(After running these commands, the certificate/signed/ directory will contain two singed 
certificates, with each having signed the other
#	Shell script to generate SSL certificate signing request using private key
#	by: Ammar Abu Shamleh

#	Variable definitions
set KEY = “”
set REQUEST = “”

#	Ask user to input filename of private key
echo “Enter the filename of the private key :”
read KEY

#	Ask user to input the desired filename of the request file
echo “Enter the output name of the request file : “
read REQUEST


#	Generate request using specified key
openssl req -new -sha256 -key $KEY -out $REQUEST

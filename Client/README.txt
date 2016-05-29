#	Basic instructions for compilation and operation of client program
#	by: Ammar Abu Shamleh

Simply run makefile to compile all code
To run client, simply directly run the oldTrustyClient application from command shell

Example:
make
./oldTrustyClient [command line ops/args]


All command line options/arguments listed on the 3002 project specifications page are 
supported

Also, please note, the Server must be run first. The Server will print out its hostname, 
and the port on which its waiting for a connection. Use these details to establish the 
connection from the client side

i.e. if Server prints hostname as “host1” and its waiting on port 8000, then a sample 
client command would be:

./oldTrustyClient -h host1:8889 -a [filename]


To remove compiled files, simply run ‘make clean’

Note: When attempting to vouch using a certificate on the server, the client application 
will look within the ‘keys’ folder to find the private key for the Server’s challenge

i.e. if I tried to vouch using certificate ‘a.crt’ on the Server, the client will look for 
‘keys/a.key’ to decrypt the Server’s challenge. If it doesn’t find this file, it will 
simply close the current session, as it can’t fulfil the challenge. This also applies when 
trying to overwrite a certificate on the Server (i.e. uploading ‘a.crt’ when ‘a.crt’ 
already exists on the Server).

PLEASE NOTE:
For the client to run, there must be a valid public-private key pair inside ‘certificates’ 
directory. These must be named ‘public.crt’ and ‘private.key’. A pair is already provided 
by default, so this note is only for those who would delete the files inside that folder.
#	Shell script to test key encryption and decryption
#	Not required to client or server, but for demonstration.
#	by: Pradyumn Vij

#	Encrypt a file using the public key like a server would do
openssl rsautl -encrypt -inkey public.key -pubin -in hello.txt -out spoon.ssl

#	Unencrypt a file using the private key like the client would
openssl rsautl -decrypt -inkey private.key -in spoon.ssl -out wow.txt
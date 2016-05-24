This file exists to test the encryption and decryption using a public-private key pair, and display the working functionality. The public-private key pair are test.crt and test.key, and the tester is Tester.java

Tester.java utilises the same encryption algorithms as Challenge.java, so this class exists to confirm that those algorithms work, and has a main method, which:
1. Generates a string 
2. Prints it
3. Encrypts it using test.crt 
4. Prints the cipher 
5. Decrypts it using test.key
6. Prints the decrypted text

Confirm the algorithms work by observing that the original plaintext and the decrypted plaintext are the same 

TESTING STEPS:
1. Copy the contents of this folder into Server
2. Run “compTester.sh”
3. Run “runTester.sh”
4. Run “cleanTester.sh” to clean all files in Server folder 
(Note: Do NOT run cleanTester.sh inside the “Sample” folder)
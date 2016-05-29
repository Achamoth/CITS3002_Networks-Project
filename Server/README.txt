#	Basic instructions for operation of Server
#	by: Ammar Abu Shamleh

This folder contains all code for the Server, and shell scripts to streamline its operation

Instructions:
1. Run the ‘init.sh’ shell script to set up Server (sets up all necessary directories, and 
compiles all code
2. After ‘init.sh’ has been run, run ‘run_server.sh’ to run Server
3. To close Server, use (cntrl+C) (default key binding to close terminal application)
4. ‘run_server.sh’ can be run again the next time you wish to launch the Server (and all 
data is preserved)


Note: ‘clean.sh’ removes all compiled code, along with any files created by the server 
during its operation, and any uploaded files/certificates. i.e. it renews the entire 
directory to before the Server was ever compiled/run

PLEASE NOTE:
For the Server to run, there must be a valid public-private key pair inside the ‘PEM’ 
directory. These files must be named ‘public.crt’ and ‘private.key’. Two such files are 
already inside that directory by default, so this note is only for those who would delete 
those files/that directory
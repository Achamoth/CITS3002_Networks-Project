#	Basic shell script to remove all operational Server files and compiled class files
#	Refreshes folder to before Server was compiled/run, effectively
#	by: Ammar Abu Shamleh

make clean
rm data
rm *.csv
rm -r Files
rm -r Certificates
rm *.key
rm iv
rm symmetric
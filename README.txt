# CS118 Project 1

Done by Richard Min and Joanne Park

Student-ids: 604-451-118 and 104450395
Please note that the specifying http protocol is mandatory for web-client.


The vagrant file has been edited so that it also installs boost automatically, which we use for parsing URIs.
Specifically, it uses cpp-netlib, which expands on boost for network libraries. 
The vagrant file also has more memory allocated to it (2 Gigs).

There is a cpp-netlib dependency; install it using ./install_cppnetlib.sh.
	If the script doesn't work make sure it's chmoded.

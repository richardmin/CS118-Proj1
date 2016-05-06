# CS118 Project 1

Done by Richard Min and Joanne Park

Student-ids: 604-451-118 and 104450395
Please note that the specifying http protocol is mandatory for web-client.


The vagrant file has been edited so that it also installs boost automatically, which we use for extra credit.
gcc 4.9 is used for the sake of regex.
The vagrant file also has more memory allocated to it (2 Gigs).

Please note that it *should* work with the vagrantfile provided, though if not installing boost is our main system dependency. sudo apt-get install -y libboost-all-dev


web-server and web-client are our base things.
web-server-timeout and web-client-timeout include HTTP/1.0 timeout. Defaults to 10 seconds.
web-server-async uses SELECT for async programming (and has no timeout)
	Note that this should break if you have a message body!


Technically speaking, not web-server-1_1 has a bug in that they might read extra characters that they're not supposed to into the logical header (i.e. the message body), but because we only support GET it doesn't matter. 
We detect the connection is closed if the file handle is stale (the correct approach would be to look at pthread_sigmask)
1.1 hasn't been tested (how do you test pipelines?)

The client makes the requests by queueing up all the send commands together and all the receive commands together.

Server timeout is supported
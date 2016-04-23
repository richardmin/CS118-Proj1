CXX=g++
CXXOPTIMIZE= -O2
BOOST=-L/vagrant/cpp-netlib-0.9.4/libs/network/src 
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE) $(BOOST) -lcppnetlib-uri -lcppnetlib-server-parsers -lcppnetlib-client-connections
USERID=EDIT_MAKE_FILE
CLASSES=HttpRequest.o HttpResponse.o

.PHONY: all
all: web-server web-client

%.o: %.h %.c
	$(CXX) $(CXXFLAGS) -c %.cpp 

web-server: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp

web-client: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp

.PHONY: clean
clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM web-server web-client a.out *.tar.gz

tarball: clean
	tar -cvf $(USERID).tar.gz *.cpp *.h Makefile README.txt
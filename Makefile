CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE)
USERID=EDIT_MAKE_FILE
CLASSES=HttpRequest.o HttpResponse.o

all: web-server web-client

%.o: %.h %.c
	$(CXX) $(CXXFLAGS) -c %.cpp 

web-server: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp

web-client: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM web-server web-client a.out *.tar.gz

tarball: clean
	tar -cvf $(USERID).tar.gz *.cpp *.h a.out Makefile 
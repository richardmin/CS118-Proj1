CXX=g++
CXXOPTIMIZE= -O0
BOOST=-lboost_regex
CXXFLAGS= -g $(CXXOPTIMIZE) -pthread -Wall -std=c++11 
LIBRARIES=$(BOOST)
USERID=604451118
CLASSES=

.PHONY: all
all: web-server web-client web-client-timeout web-server-timeout

%.o: %.h %.c
	$(CXX) $(CXXFLAGS) -c %.cpp 

web-server: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp $(LIBRARIES)

web-client: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp $(LIBRARIES)

web-server-timeout: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp $(LIBRARIES)

web-client-timeout: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp $(LIBRARIES)

.PHONY: clean
clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM web-server web-client a.out web-server-timeout web-client-timeout *.tar.gz

tarball: clean
	tar -cvf $(USERID).tar.gz *.cpp *.h Makefile README.txt

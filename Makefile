CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE)
USERID=505029637
CLASSES=

all: server client

server: server.cpp  # $(CLASSES)
	$(CXX) -o $@ $(CXXFLAGS) $@.cpp

client: client.cpp # $(CLASSES)
	$(CXX) -o $@ $(CXXFLAGS) $@.cpp

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM server client *.tar.gz

dist: tarball
tarball: clean
	tar -cvzf /tmp/$(USERID).tar.gz --exclude=./.vagrant . && mv /tmp/$(USERID).tar.gz .

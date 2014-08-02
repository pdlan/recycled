CXX=clang++
INCLUDE=../include
CXXFLAGS=-std=c++11 -Wall -I $(INCLUDE)
all: ioloop.o httpserver.o httpconnection.o test
ioloop.o: $(INCLUDE)/recycled/ioloop.h ioloop.cpp
	$(CXX) $(CXXFLAGS) ioloop.cpp -c
httpserver.o: ioloop.o $(INCLUDE)/recycled/httpserver.h $(INCLUDE)/recycled/connection.h httpserver.cpp
	$(CXX) $(CXXFLAGS) httpserver.cpp -c
httpconnection.o: $(INCLUDE)/recycled/httpconnection.h $(INCLUDE)/recycled/connection.h httpconnection.cpp
	$(CXX) $(CXXFLAGS) httpconnection.cpp -c
test: ioloop.o httpserver.o httpconnection.o
	$(CXX) $(CXXFLAGS) test.cpp *.o -o test -levent
clean:
	rm *.o test
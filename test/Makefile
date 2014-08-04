CXX=g++
INCLUDE=../include
CXXFLAGS=-std=c++11 -Wall -I $(INCLUDE)
all: format hello
format:
	$(CXX) $(CXXFLAGS) format.cpp -o format.test
hello:
	$(CXX) $(CXXFLAGS) hello.cpp -o hello.test ../librecycled.a -lpcre -levent
clean:
	rm *.test
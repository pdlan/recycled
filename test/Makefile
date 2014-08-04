CXX=clang++
INCLUDE=../include
CXXFLAGS=-std=c++11 -Wall -I $(INCLUDE)
all: format hello
format: format.cpp
	$(CXX) $(CXXFLAGS) format.cpp -o format.test
hello: hello.cpp
	$(CXX) $(CXXFLAGS) hello.cpp -o hello.test ../librecycled.a -lpcre -levent
clean:
	rm *.test
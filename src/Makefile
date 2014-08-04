CXX=clang++
INCLUDE=../include
CXXFLAGS=-std=c++11 -Wall -I $(INCLUDE)
all: ioloop.o httpserver.o httpconnection.o recycled
headers: $(INCLUDE)/recycled/*.h
ioloop.o: headers ioloop.cpp
	$(CXX) $(CXXFLAGS) ioloop.cpp -c
httpserver.o: headers httpserver.cpp
	$(CXX) $(CXXFLAGS) httpserver.cpp -c
httpconnection.o: headers httpconnection.cpp
	$(CXX) $(CXXFLAGS) httpconnection.cpp -c
router.o: headers router.cpp
	$(CXX) $(CXXFLAGS) router.cpp -c
recycled: ioloop.o httpserver.o httpconnection.o router.o
	ar rcs librecycled.a ioloop.o httpserver.o httpconnection.o router.o
	mv librecycled.a ..
clean:
	rm *.o ../librecycled.a
all: format connection
format:
	@echo "Testing format utility"
	clang++ format.cpp -I ../include/ -std=c++11 -o format.test -g -Wall
	./format.test
connection:
	@echo "Testing connection"
	clang++ connection.cpp -I ../include/ -std=c++11 -o connection.test -g -Wall
	./connection.test
clean:
	rm *.test
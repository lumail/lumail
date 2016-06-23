#
# Simple Makefile, could be improved
#


all: build test

build: $(wildcard *.cc) $(wildcard *.c)
	g++ -std=c++0x -Wall -Werror -o test-suite -I../src/ *.c *.cc -lpcrecpp

clean:
	rm test-suite || true

test:
	./test-suite

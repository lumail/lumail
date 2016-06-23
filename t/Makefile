#
# Simple Makefile, could be improved
#


all: build test

build: $(wildcard *.cc) $(wildcard *.c)
	g++ -o test-suite -I../src/ *.c *.cc ../src/colour_string.cc -lpcrecpp

clean:
	rm test-suite || true

test:
	./test-suite

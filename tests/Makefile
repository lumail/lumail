#
#  Simple Makefile for test-cases.
#
#


#
#  Build the test-binaries.
#
all: file_tests history_tests


#
#  Run the tests-binaries
#
test: all
	./file_tests
	./history_tests


#
#  Cleanup the generated files.
#
clean:
	rm -f file_tests history_tests || true


#
#  Build the various test-binaries.
#

file_tests: file_tests.cpp ../file.cc
	g++ -I.. -o file_tests ../file.cc file_tests.cpp

history_tests: history_tests.cpp ../history.cc
	g++ -I.. -o history_tests ../history.cc history_tests.cpp
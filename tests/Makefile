#
#  Simple Makefile for test-cases.
#
#


all: file_tests history_tests


clean:
	rm -f file_tests history_tests || true


file_tests: file_tests.cpp ../file.cc
	g++ -I.. -o file_tests ../file.cc file_tests.cpp

history_tests: history_tests.cpp ../history.cc
	g++ -I.. -o history_tests ../history.cc history_tests.cpp
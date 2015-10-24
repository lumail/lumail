
#
#  Basics
#
C=gcc
CC=g++
LINKER=$(CC) -o


CPPFLAGS+=-std=gnu++0x -Wall -Werror  $(shell pkg-config --cflags ncursesw)
LDLIBS+=$(shell pkg-config --libs ncursesw) -lpanelw

#
# Default targets
#
default: ui-demo

#
#  Cleanup
#
clean:
	rm ui-demo || true

#
#  The release-build.
#
ui-demo: ui-demo.c
	g++ -o ui-demo ui-demo.c $(CPPFLAGS) $(LDLIBS)

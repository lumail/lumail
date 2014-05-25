#
# Compiler + Flags
#
C=gcc
CFLAGS=-Wall -Werror

CC=g++
CCFLAGS=-rdynamic -std=gnu++0x


#
#  All of our targets
#
all: attachments add-attachments-to-simple-mail dump-headers dump-mime dump-parts lumailctl parse-fmt


#
#  Cleanup all generated binaries and output-files.
#
clean:
	rm attachments add-attachments-to-simple-mail dump-headers dump-mime dump-parts lumailctl parse-fmt || true
	rm input.txt message.out || true


#
#  Targets follow
#

attachments: attachments.c Makefile
	 $(CC) $(CCFLAGS)  -I../src/ attachments.c -o attachments $(shell pkg-config --libs  gmime-2.6) $(shell pkg-config --cflags gmime-2.6)  $(shell pkg-config --libs  glibmm-2.4) $(shell pkg-config --cflags glibmm-2.4)


add-attachments-to-simple-mail: add-attachments-to-simple-mail.c Makefile
	 $(CC) $(CCFLAGS) add-attachments-to-simple-mail.c -o add-attachments-to-simple-mail $(shell pkg-config --libs  gmime-2.6) $(shell pkg-config --cflags gmime-2.6)

dump-headers: dump-headers.c Makefile
	 $(CC) $(CCFLAGS) dump-headers.c -o dump-headers $(shell pkg-config --libs  gmime-2.6) $(shell pkg-config --cflags gmime-2.6)

dump-mime: dump-mime.c Makefile
	 $(CC) $(CCFLAGS)  dump-mime.c -o dump-mime $(shell pkg-config --libs  gmime-2.6) $(shell pkg-config --cflags gmime-2.6)

dump-parts: dump-parts.c Makefile
	 $(CC) $(CCFLAGS) dump-parts.c -o dump-parts $(shell pkg-config --libs  gmime-2.6) $(shell pkg-config --cflags gmime-2.6)

lumailctl: lumailctl.c Makefile
	 $(C) $(CFLAGS) -I../src/ lumailctl.c -o lumailctl

parse-fmt: parse-fmt.cc Makefile
	 $(CC) $(CCFLAGS) parse-fmt.cc -o parse-fmt $(shell pkg-config --libs  gmime-2.6) $(shell pkg-config --cflags gmime-2.6) -lpcrecpp



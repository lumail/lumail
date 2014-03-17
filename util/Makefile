


all: add-attachments-to-simple-mail dump-headers dump-mime dump-parts lumailctl parse-fmt


clean:
	rm add-attachments-to-simple-mail dump-headers dump-mime dump-parts lumailctl parse-fmt || true
	rm input.txt message.out || true


dump-headers: dump-headers.c Makefile
	 g++ -ggdb -rdynamic dump-headers.c -o dump-headers $(shell pkg-config --libs  gmime-2.6) $(shell pkg-config --cflags gmime-2.6)

dump-mime: dump-mime.c Makefile
	 g++ -ggdb -rdynamic dump-mime.c -o dump-mime $(shell pkg-config --libs  gmime-2.6) $(shell pkg-config --cflags gmime-2.6)

dump-parts: dump-parts.c Makefile
	 g++ -ggdb -rdynamic dump-parts.c -o dump-parts $(shell pkg-config --libs  gmime-2.6) $(shell pkg-config --cflags gmime-2.6)

lumailctl: lumailctl.c Makefile
	 gcc -Wall -ggdb lumailctl.c -o lumailctl

parse-fmt: parse-fmt.cc Makefile
	 g++ -std=gnu++0x -ggdb -rdynamic parse-fmt.cc -o parse-fmt $(shell pkg-config --libs  gmime-2.6) $(shell pkg-config --cflags gmime-2.6) -lpcrecpp



add-attachments-to-simple-mail: add-attachments-to-simple-mail.c Makefile
	 g++ -std=gnu++0x -ggdb add-attachments-to-simple-mail.c -o add-attachments-to-simple-mail $(shell pkg-config --libs  gmime-2.6) $(shell pkg-config --cflags gmime-2.6)


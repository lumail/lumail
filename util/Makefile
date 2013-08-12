


all: add-attachments-to-simple-mail  dump-mime


clean:
	rm add-attachments-to-simple-mail  dump-mime || true
	rm input.txt message.out || true


dump-mime: dump-mime.c Makefile
	 g++ -ggdb -rdynamic dump-mime.c -o dump-mime $(shell pkg-config --libs  gmime-2.6) $(shell pkg-config --cflags gmime-2.6)



add-attachments-to-simple-mail: add-attachments-to-simple-mail.c Makefile
	 g++ -ggdb add-attachments-to-simple-mail.c -o add-attachments-to-simple-mail $(shell pkg-config --libs  gmime-2.6) $(shell pkg-config --cflags gmime-2.6)


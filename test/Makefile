ALL: array-to-lua body readline str

array-to-lua: array-to-lua.cc
	g++ -o array-to-lua array-to-lua.cc $(shell pkg-config --cflags --libs lua5.1)

lua-to-c: lua-to-c.cc
	g++ -o lua-to-c lua-to-c.cc $(shell pkg-config --cflags --libs lua5.1)

body: body.cc Makefile
	g++ -o body body.cc -lmimetic

readline: readline.cc Makefile
	g++ -o readline readline.cc -lreadline

str: str.cc Makefile
	g++ -o str str.cc

clean:
	rm -f array-to-lua body str readline || true

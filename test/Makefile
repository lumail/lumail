ALL: body readline str

body: body.cc Makefile
	g++ -o body body.cc -lmimetic

readline: readline.cc Makefile
	g++ -o readline readline.cc -lreadline

str: str.cc Makefile
	g++ -o str str.cc

clean:
	rm -f body str readline || true

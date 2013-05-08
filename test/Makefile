ALL: str body

str: str.cc Makefile
	g++ -o str str.cc

body: body.cc Makefile
	g++ -o body body.cc -lmimetic

clean:
	rm -f str body || true

ALL: array-to-lua body i18n readline str table

array-to-lua: array-to-lua.cc
	g++ -o array-to-lua array-to-lua.cc $(shell pkg-config --cflags --libs lua5.1)

lua-to-c: lua-to-c.cc
	g++ -o lua-to-c lua-to-c.cc $(shell pkg-config --cflags --libs lua5.1)

body: body.cc Makefile
	g++ -o body body.cc -lmimetic

i18n: i18n.cc Makefile
	g++ -o i18n i18n.cc
	xgettext --package-name i18n --package-version 0.1 --default-domain i18n --output i18n.pot i18n.cc
	msginit --no-translator --locale es_MX --output-file i18n_spanish.po --input i18n.pot
	sed --in-place i18n_spanish.po --expression='/"hello, world!"/,/#: / s/""/"hola mundo"/'
	mkdir --parents ./es_MX.utf8/LC_MESSAGES
	msgfmt --check --verbose --output-file ./es_MX.utf8/LC_MESSAGES/i18n.mo i18n_spanish.po
	LANGUAGE=es_MX.utf8 ./i18n

readline: readline.cc Makefile
	g++ -o readline readline.cc -lreadline

str: str.cc Makefile
	g++ -o str str.cc

table: table.cc
	g++ -o table table.cc $(shell pkg-config --cflags --libs lua5.1)

clean:
	rm -f core array-to-lua body str readline table || true
	rm -rf es_MX.utf8/ i18n i18n.pot i18n_spanish.po || true

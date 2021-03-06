#
# This is the Makefile for compiling lumail2.
#
# It is tested to work upon Linux and Mac OS X, and may well work
# upon other systems.
#
# The main configuration you'll need to look at here is the version
# of Lua which you're compiling against.
#
# We default to Lua 5.2, but 5.1 and 5.3 are also expected to work.
#
# If you struggle to compile this on a "common" system please do report
# a bug against the project:
#
#   https://github.com/lumail/lumail
#
#
# Steve
# --
#
#

#
# The version of our code - in release-tarballs this is hardcoded,
# in the git repository it is the hash of the current commit.
#
VERSION=$(shell git describe --abbrev=4 --dirty --always --long)

#
# Install locations
#
PKGNAME?=lumail
DESTDIR?=
PREFIX?=/usr
SYSCONFDIR?=/etc
LUMAIL_ETC?=$(DESTDIR)$(SYSCONFDIR)/lumail
LUMAIL_HOME?=$(DESTDIR)$(PREFIX)/share/lumail
LUMAIL_LIBS?=$(DESTDIR)$(PREFIX)/lib/lumail
DOCDIR?=$(DESTDIR)$(PREFIX)/share/doc/$(PKGNAME)
LUMAIL_HOME_OLD?=$(DESTDIR)$(SYSCONFDIR)/lumail2
LUMAIL_LIBS_OLD?=${LUMAIL_HOME_OLD}/lib

#
# Load the flags if they're not already set - first look at the version
#
LUA_VERSION?=5.2
LVER?=lua$(LUA_VERSION)
UNAME := $(shell uname -s)
ifeq ($(UNAME),DragonFly)
	LVER=lua-$(LUA_VERSION)
endif
ifeq ($(UNAME),Darwin)
	LVER=lua #(use lua-52)
endif

#
# To ensure make finds the ncursesw.h header file,
# you may need to invoke it like this:
#   PKG_CONFIG_PATH=/usr/local/Cellar/ncurses/6.0_1/lib/pkgconfig make
# PKG_CONFIG_PATH should point to a directory containing a file
# named ncursesw.pc)
#
ifeq ($(UNAME),Darwin)
   C=clang
   CC=clang++
   CPPFLAGS+=-I /usr/include/malloc
endif



#
# Now we know the version of Lua we'll setup the appropriate
# compilation & linker flags here.
#
LUA_FLAGS ?= $(shell pkg-config --cflags ${LVER})
LUA_LIBS  ?= $(shell pkg-config --libs ${LVER})


#
# If you prefer you can use a manual set of flags, explicitly.
#
# This is useful for testing against local versions of Lua.
#
# If these lines are commented-out then we'll discover them dynamically
# via `pkg-config` setup above.
#
# LUA_FLAGS=-I/home/steve/Downloads/lua-5.3.2/src
# LUA_LIBS=-L/home/steve/Downloads/lua-5.3.2/install/lib -llua
#

#
# Finally if we prefer we can use luajit, via one of these options:
#
### Locally compiled version of luajit.
#LUA_FLAGS=-I/tmp/luajit/include/luajit-2.1/
#LUA_LIBS=-L/tmp/luajit/lib/ -lluajit-5.1
#
### Debian packaged-version of luajit.
#LUA_FLAGS = $(shell pkg-config --cflags luajit)
#LUA_LIBS  = $(shell pkg-config --libs luajit)



#
#  Source + Object + Binary directories
#
SRCDIR         = src
RELEASE_OBJDIR = obj.release
DEBUG_OBJDIR   = obj.debug


#
#  Basics
#
CC?=g++
LINKER=$(CC) -o



#
# Compilation flags and setup for packages we use.
#
CPPFLAGS+=-Wall -Werror
override CPPFLAGS+=-std=c++0x
override CPPFLAGS+=-DLUMAIL_VERSION="\"${VERSION}\"" -DLUMAIL_LUAPATH="\"${LUMAIL_LIBS}\""
override CPPFLAGS+=${LUA_FLAGS} $(shell pcre-config --cflags) $(shell pkg-config --cflags ncursesw) $(shell pkg-config --cflags gmime-2.6)

#
# Linker flags for the packages we use.
#
LDLIBS+=${LUA_LIBS} $(shell pkg-config --libs gmime-2.6) $(shell pkg-config --libs ncursesw) $(shell pkg-config --libs panelw)
LDLIBS+=-lpcrecpp -lmagic -lstdc++ -lm



#
#  Only build the release-target by default.
#
default: lumail2


#
#  Cleanup
#
clean:
	test -d docs/              && rm -rf docs              || true
	test -d $(RELEASE_OBJDIR)  && rm -rf $(RELEASE_OBJDIR) || true
	test -d $(DEBUG_OBJDIR)    && rm -rf $(DEBUG_OBJDIR)   || true
	rm -f gmon.out lumail2 lumail2-debug core              || true
	find . -name '*.orig' -delete                          || true


#
#  Sources + objects.
#
SOURCES         := $(wildcard $(SRCDIR)/*.cc)
RELEASE_OBJECTS := $(SOURCES:$(SRCDIR)/%.cc=$(RELEASE_OBJDIR)/%.o)
DEBUG_OBJECTS   := $(SOURCES:$(SRCDIR)/%.cc=$(DEBUG_OBJDIR)/%.o)


#
#  The release-build.
#
lumail2: $(RELEASE_OBJECTS)
	$(LINKER) $@ $(LFLAGS) $(RELEASE_OBJECTS) $(LDLIBS) $(GMIME_LIBS)

#
#  The debug-build.
#
lumail2-debug: $(DEBUG_OBJECTS)
	$(LINKER) $@ $(LFLAGS) -rdynamic -ggdb -pg $(DEBUG_OBJECTS) $(LDLIBS) $(GMIME_LIBS)


#
#  Build the objects for the release build.
#
$(RELEASE_OBJECTS): $(RELEASE_OBJDIR)/%.o : $(SRCDIR)/%.cc
	@mkdir $(RELEASE_OBJDIR) 2>/dev/null || true
	$(CC) $(CPPFLAGS) $(GMIME_INC) -O2 -c $< -o $@

#
#  Build the objects for the debug build - which has an extra definition and
# a different object directory.
#
$(DEBUG_OBJECTS): $(DEBUG_OBJDIR)/%.o : $(SRCDIR)/%.cc
	@mkdir $(DEBUG_OBJDIR) 2>/dev/null || true
	$(CC) -ggdb -DDEBUG=1 $(CPPFLAGS) $(GMIME_INC) -O2 -c $< -o $@


#
# Indent our C++ code in a consistent-style
#
.PHONY: indent
indent:
	astyle --style=allman -A1 --indent=spaces=4   --break-blocks --pad-oper --pad-header --unpad-paren --max-code-length=200 $(SRCDIR)/*.cc $(SRCDIR)/*.h


#
# Indent our Lua code in a consistent-style.
#
.PHONY: indent-lua
indent-lua:
	lunadry.lua --in-place $$(find . -name '*.lua' -type f)


#
# Rebuild our (code) documentation.
#
.PHONY: docs
docs:
	doxygen


#
# Serve our documentation via a local python HTTP-server.
#
.PHONY: serve_docs
serve_docs: docs
	echo "Visit http://127.0.0.1:8000/"
	cd docs/html && python -m SimpleHTTPServer


#
# Run our test-cases, from the main binary.
#
test: lumail2
	./lumail2 --test


#
# Run our lua test-cases
#
test-lua: lumail2
	for i in t/test*.lua; do ./lumail2 --no-default --load-file $$i --no-curses || exit 1; done


#
#  Cleanup obsolete versions of our IMAP code
#
clean_imap:
	rm -rf $(LUMAIL_HOME_OLD)/perl.d/


#
#  Install the IMAP proxy beneath /usr/share/lumail
#
install_imap: clean_imap
	mkdir -p $(LUMAIL_HOME) || true
	install -m755 perl.d/imap-proxy $(LUMAIL_HOME)
	install -m644 perl.d/Lumail.pm  $(LUMAIL_HOME)


#
# Install our Lua libraries - make a backup of the global
# configuration-file, in case the user has editted it.
#
install_lua:
	mkdir -p $(LUMAIL_LIBS) || true
	install -m644 lib/*.lua $(LUMAIL_LIBS)
	mkdir -p $(LUMAIL_ETC) || true
	mv $(LUMAIL_ETC)/lumail.lua $(LUMAIL_ETC)/lumail.lua.$$(date +%d-%m-%Y.%s) || true
	install -m644 ./global.config.lua $(LUMAIL_ETC)/lumail.lua

#
# Install documentation files.
#
install_doc:
	mkdir -p $(DOCDIR) || true
	install -m644 ./*.md $(DOCDIR)

#
# Install tests and sample files.
#
install_data:
	install -m644 ./user.config.lua $(LUMAIL_HOME)
	mkdir -p $(LUMAIL_HOME)/tests || true
	for test in t/*; do \
		install -m644 $$test $(LUMAIL_HOME)/tests ;\
	done
	mkdir -p $(LUMAIL_HOME)/sample.lua || true
	for sample in sample.lua/*; do \
		install -m644 $$sample $(LUMAIL_HOME)/sample.lua ;\
	done

#
#  Install the binary, and use the other targets to install
# the Lua libraries and perl proxy.
#
install: lumail2 install_imap install_lua
	mkdir -p $(DESTDIR)$(PREFIX)/bin || true
	install -m755 lumail2 $(DESTDIR)$(PREFIX)/bin/


#
#  Test for leaks; use the debug-build so we get line-number information, etc.
#
valgrind: lumail2-debug
	valgrind  --leak-check=full --show-leak-kinds=all --track-origins=yes ./lumail2-debug  --load-file ./global.config.lua 2>leak.log

#
# The version of our code.
#
VERSION=$(shell git describe --abbrev=4 --dirty --always --long)


#
# Here we setup the path to our Lua include-files and libraries.
#
# This is specifically for testing against local versions of Lua.
#
# If these lines are commented-out then we'll discover them dynamically
# via `pkg-config`.
#
# LUA_FLAGS=-I/home/steve/Downloads/lua-5.3.2/src
# LUA_LIBS=-L/home/steve/Downloads/lua-5.3.2/install/lib -llua -ldl


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
# Then actually set the flags.
#
LUA_FLAGS ?= $(shell pkg-config --cflags ${LVER})
LUA_LIBS  ?= $(shell pkg-config --libs ${LVER})


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
# Compilation flags and libraries we use.
#
CPPFLAGS+=${LUA_FLAGS} -std=c++0x -Wall -Werror  $(shell pcre-config --cflags) $(shell pkg-config --cflags ncursesw) -DLUMAIL_VERSION="\"${VERSION}\""
LDLIBS+=${LUA_LIBS} $(shell pkg-config --libs ncursesw) $(shell pkg-config --libs panelw) -lpcrecpp -lmagic -ldl -lstdc++

#
#  GMime is used for MIME handling.
#
GMIME_LIBS=$(shell pkg-config --libs  gmime-2.6)
GMIME_INC=$(shell pkg-config --cflags gmime-2.6)

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
	astyle --style=allman -A1 --indent=spaces=4   --break-blocks --pad-oper --pad-header --unpad-paren --max-code-length=200 $(SRCDIR)/*.cc $(SRCDIR)/*.h t/*.cc


#
# Rebuild our (code) documentation.
#
.PHONY: docs
docs:
	doxygen

#
# Serve our documentation via a local python HTTP-server.
#
.PHONE: serve_docs
serve_docs: docs
	echo "Visit http://127.0.0.1:8000/"
	cd docs/html && python -m SimpleHTTPServer



test:
	cd t/ && make

#
#  Install the binary, and our luarocks.d directory
#
install: lumail2
	cp lumail2 /usr/bin/

    # make target-directories
	mkdir -p /etc/lumail2/luarocks.d/  || true
	mkdir -p /etc/lumail2/perl.d/  || true

    # copy our helpers
	cp luarocks.d/*.lua /etc/lumail2/luarocks.d/
	cp perl.d/* /etc/lumail2/perl.d/

    # cleanup old installs
	rm /etc/lumail2/perl.d/delete-message || true
	rm /etc/lumail2/perl.d/get-folders || true
	rm /etc/lumail2/perl.d/get-messages || true
	rm /etc/lumail2/perl.d/save-message || true
	rm /etc/lumail2/perl.d/set-flags || true

    # if there is no config in-place, add the default
	if [ ! -e /etc/lumail2/lumail2.lua ] ; then cp ./lumail2.lua /etc/lumail2/lumail2.lua ; fi

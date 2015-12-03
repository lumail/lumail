#
# The version of our code.
#
VERSION=$(shell git describe --abbrev=4 --dirty --always --long)


#
# We've tested compilation with Lua 5.2 only
#
LUA_VERSION?=5.2


#
#  Source + Object + Binary directories
#
SRCDIR         = src
RELEASE_OBJDIR = obj.release
DEBUG_OBJDIR   = obj.debug


#
#  Basics
#
C=gcc
CC=g++
LINKER=$(CC) -o


#
# The name of the library we'll link against differs on different
# systems, which is fun.
#
LVER?=lua$(LUA_VERSION)
UNAME := $(shell uname -s)
ifeq ($(UNAME),DragonFly)
	LVER=lua-$(LUA_VERSION)
endif
ifeq ($(UNAME),Darwin)
	LVER=lua #(use lua-52)
endif


#
# Compilation flags and libraries we use.
#
CPPFLAGS+=-std=c++0x -Wall -Werror $(shell pkg-config --cflags ${LVER}) $(shell pcre-config --cflags) $(shell pkg-config --cflags ncursesw) -DLUMAIL_VERSION="\"${VERSION}\""
LDLIBS+=$(shell pkg-config --libs ${LVER}) $(shell pkg-config --libs ncursesw) $(shell pkg-config --libs panelw) -lpcrecpp -lmagic

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
	$(LINKER) $@ $(LFLAGS) $(RELEASE_OBJECTS) $(LDLIBS) $(GMIME_LIBS) $(GLIBMM_LIBS)

#
#  The debug-build.
#
lumail2-debug: $(DEBUG_OBJECTS)
	$(LINKER) $@ $(LFLAGS) -rdynamic -ggdb -pg $(DEBUG_OBJECTS) $(LDLIBS) $(GMIME_LIBS) $(GLIBMM_LIBS)


#
#  Build the objects for the release build.
#
$(RELEASE_OBJECTS): $(RELEASE_OBJDIR)/%.o : $(SRCDIR)/%.cc
	@mkdir $(RELEASE_OBJDIR) 2>/dev/null || true
	$(CC) $(FEATURES) $(CPPFLAGS) $(GMIME_INC) $(GLIBMM_INC) -O2 -c $< -o $@

#
#  Build the objects for the debug build.
#
#  Just define "LUMAIL_DEBUG=1", otherwise share the flags.
#
$(DEBUG_OBJECTS): $(DEBUG_OBJDIR)/%.o : $(SRCDIR)/%.cc
	@mkdir $(DEBUG_OBJDIR) 2>/dev/null || true
	$(CC) $(FEATURES) -ggdb -DDEBUG=1 $(CPPFLAGS) $(GMIME_INC) $(GLIBMM_INC) -O2 -c $< -o $@


#
# Indent our C++ code in a consistent-style
#
.PHONY: indent
indent:
	astyle --style=allman -A1 --indent=spaces=4   --break-blocks --pad-oper --pad-header --unpad-paren --max-code-length=200 $(SRCDIR)/*.cc $(SRCDIR)/*.h


#
# rebuild our (code) documentation
#
.PHONY: docs
docs:
	doxygen


#
#  Install the binary, and our luarocks.d directory
#
install: lumail2
	cp lumail2 /usr/bin/
	mkdir -p /etc/lumail2/luarocks.d/  || true
	cp luarocks.d/*.lua /etc/lumail2/luarocks.d/

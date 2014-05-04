##
##  This is the Makefile for lumail.
##
##  It is a little more complex than it should be because of the main requirement
## that we build both "lumail" and "lumail-debug" with the same sources.
##
##  Because the debug-version of the code requires different CPPFLAGS we cannot
## just compile once.  So instead we must have two object directories; one for each
## set of flags.
##
##  To cut down on manual updates though we look for src/*.cc and compile everything
## en mass.
##
##  If you're running `ccache` then the overhead of double-compilation
## should be insignificant.
##
##  Steve
##  --
##



#
# Features which can be compiled in/out
#
FEATURES=-DDOMAIN_SOCKET=1

#
# We've tested compilation with Lua 5.1 and 5.2.
#
LUA_VERSION=5.1


#
#  Source + Object + Binary directories
#
SRCDIR         = src
RELEASE_OBJDIR = obj.release
DEBUG_OBJDIR   = obj.debug


#
#  Used solely for building a new release tarball.  ("make release").
#
#  The version comes from the convention that Steve tags the releases
# with "release-N.N".
#
TMP?=/tmp
BASE=lumail
DIST_PREFIX=${TMP}
VERSION=$(shell sh -c 'git describe --abbrev=0 --tags | tr -d "release-"')

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
LVER=lua$(LUA_VERSION)
UNAME := $(shell uname -s)
ifeq ($(UNAME),DragonFly)
LVER=lua-$(LUA_VERSION)
endif


#
# Compilation flags and libraries we use.
#
CPPFLAGS+=-std=gnu++0x -Wall -Werror $(shell pkg-config --cflags ${LVER}) $(shell pcre-config --cflags) -I/usr/include/ncursesw/
LDLIBS+=$(shell pkg-config --libs ${LVER}) -lncursesw -lpcrecpp

#
#  GMime is used for MIME handling.
#
GMIME_LIBS=$(shell pkg-config --libs  gmime-2.6)
GMIME_INC=$(shell pkg-config --cflags gmime-2.6)

#
# UTF-8-aware string handling.
#
GLIBMM_LIBS=$(shell pkg-config --libs  glibmm-2.4)
GLIBMM_INC=$(shell pkg-config --cflags glibmm-2.4)

#
#  Build both targets by default, along with the helper.
#
default: lumail lumail-debug lumailctl


#
#  Style-check our code
#
.PHONY: style
style:
	prove --shuffle ./style/

#
#  Make a release
#
release: clean style
	rm -rf $(DIST_PREFIX)/$(BASE)-$(VERSION)
	rm -f $(DIST_PREFIX)/$(BASE)-$(VERSION).tar.gz
	cp -R . $(DIST_PREFIX)/$(BASE)-$(VERSION)
	rm -rf $(DIST_PREFIX)/$(BASE)-$(VERSION)/debian
	rm -rf $(DIST_PREFIX)/$(BASE)-$(VERSION)/.git*
	rm -rf $(DIST_PREFIX)/$(BASE)-$(VERSION)/.depend || true
	perl -pi -e "s/__UNRELEASED__/$(VERSION)/g" $(DIST_PREFIX)/$(BASE)-$(VERSION)/src/version.h
	perl -pi -e "s/__UNRELEASED__/$(VERSION)/g" $(DIST_PREFIX)/$(BASE)-$(VERSION)/lumail.help
	cd $(DIST_PREFIX) && tar -cvf $(DIST_PREFIX)/$(BASE)-$(VERSION).tar $(BASE)-$(VERSION)/
	gzip $(DIST_PREFIX)/$(BASE)-$(VERSION).tar
	mv $(DIST_PREFIX)/$(BASE)-$(VERSION).tar.gz .
	rm -rf $(DIST_PREFIX)/$(BASE)-$(VERSION)



#
#  Cleanup
#
clean:
	test -d $(RELEASE_OBJDIR)  && rm -rf $(RELEASE_OBJDIR) || true
	test -d $(DEBUG_OBJDIR)    && rm -rf $(DEBUG_OBJDIR)   || true
	rm -f gmon.out lumail lumail-debug lumailctl core      || true
	@cd util && make clean                                 || true


#
#  Sources + objects.
#
SOURCES         := $(wildcard $(SRCDIR)/*.cc)
RELEASE_OBJECTS := $(SOURCES:$(SRCDIR)/%.cc=$(RELEASE_OBJDIR)/%.o)
DEBUG_OBJECTS   := $(SOURCES:$(SRCDIR)/%.cc=$(DEBUG_OBJDIR)/%.o)


#
#  The release-build.
#
lumail: $(RELEASE_OBJECTS)
	$(LINKER) $@ $(LFLAGS) $(RELEASE_OBJECTS) $(LDLIBS) $(GMIME_LIBS) $(GLIBMM_LIBS)

#
#  The debug-build.
#
lumail-debug: $(DEBUG_OBJECTS)
	$(LINKER) $@ $(LFLAGS) -rdynamic -ggdb -pg $(DEBUG_OBJECTS) $(LDLIBS) $(GMIME_LIBS) $(GLIBMM_LIBS)


#
# The domain-socket helper
#
lumailctl:
	$(C) -Wall util/lumailctl.c -o lumailctl


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
	$(CC) $(FEATURES) -ggdb -DLUMAIL_DEBUG=1 $(CPPFLAGS) $(GMIME_INC) $(GLIBMM_INC) -O2 -c $< -o $@

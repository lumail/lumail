#
#  Makefile for lumail, the console mail client.  Further details online
# at http://lumail.org/
#



#
#  Only used to build distribution tarballs.
#
BASE        = lumail
DIST_PREFIX = ${TMP}
VERSION     = $(shell sh -c 'git describe --abbrev=0 --tags | tr -d "release-"')


#
#  Source objects.
#
SRCS= bindings.cc debug.cc file.cc global.cc history.cc lua.cc maildir.cc message.cc main.cc screen.cc
OBJS=$(subst .cc,.o,$(SRCS))
TARGET=lumail

#
#  The version of Lua we're building against.
#  Valid options are "lua5.1" & "lua5.2".
#
LVER=lua5.1


#
# Flags.
#
# NOTE: We use "-std=gnu++0x" so we can use "unordered_map".
#
CPPFLAGS+=-std=gnu++0x -g -Wall -Werror $(shell pkg-config --cflags ${LVER})
LDLIBS+=$(shell pkg-config --libs ${LVER})  $(shell pcre-config --libs-cpp)  -lcurses -lmimetic


#
# Default target.
#
all: $(TARGET)


#
# Debug target.
#
lumail-debug: CXX += -DLUMAIL_DEBUG=1
lumail-debug: TARGET=lumail-debug
lumail-debug: $(TARGET)


#
#  Build the target
#
$(TARGET): $(OBJS)
	$(CXX) $(CPPFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LDLIBS)


#
#  Dependency generation.
#
depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) $(LDLIBS) -MM $^>>./.depend;


#
# Cleanup
#
clean:
	$(RM) $(TARGET) lumail-debug $(OBJS) core || true
	cd ./tests && make clean || true

dist-clean: clean
	$(RM) *~ .dependtool


#
#  Install to /usr/local.
#
install: all
	if [ -e /etc/lumail.lua ]; then mv /etc/lumail.lua /etc/lumail.lua.old ; fi
	cp ./lumail.lua /etc/lumail.lua
	cp ./lumail /usr/local/bin


#
#  Make a release tarball
#
release: clean
	rm -rf $(DIST_PREFIX)/$(BASE)-$(VERSION)
	rm -f $(DIST_PREFIX)/$(BASE)-$(VERSION).tar.gz
	cp -R . $(DIST_PREFIX)/$(BASE)-$(VERSION)
	rm -rf $(DIST_PREFIX)/$(BASE)-$(VERSION)/debian
	rm -rf $(DIST_PREFIX)/$(BASE)-$(VERSION)/.git*
	perl -pi -e "s/__UNRELEASED__/$(VERSION)/g" $(DIST_PREFIX)/$(BASE)-$(VERSION)/version.h
	cd $(DIST_PREFIX) && tar -cvf $(DIST_PREFIX)/$(BASE)-$(VERSION).tar $(BASE)-$(VERSION)/
	gzip $(DIST_PREFIX)/$(BASE)-$(VERSION).tar
	mv $(DIST_PREFIX)/$(BASE)-$(VERSION).tar.gz .
	rm -rf $(DIST_PREFIX)/$(BASE)-$(VERSION)
#	gpg --armour --detach-sign $(BASE)-$(VERSION).tar.gz
#	echo $(VERSION) > .version


include .depend

#
#  Source objects.
#
SRCS= bindings.cc global.cc lua.cc maildir.cc message.cc main.cc screen.cc
OBJS=$(subst .cc,.o,$(SRCS))
CPPFLAGS=-g -Wall -Werror $(shell pkg-config --cflags lua5.1)
LDLIBS=$(shell pkg-config --libs lua5.1) -lcurses -lmimetic
TARGET=lumail

all: $(TARGET)

$(TARGET): $(OBJS)
	g++ $(CPPFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LDLIBS)

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) $(LDLIBS) -MM $^>>./.depend;

clean:
	$(RM) $(TARGET) $(OBJS)

dist-clean: clean
	$(RM) *~ .dependtool

tidy:
	indent -linux -i4 *.cc *.h


include .depend

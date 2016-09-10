
HSMP		= ../../src/hsmp
HSMPFLAGS	= -i -v -d -S
CC		= gcc
CCFLAGS		= -g -Wall
LDFLAGS		= -lpthread

all: keypress keypress2 keypress3

keypress:	keypress.c
	$(CC) $(CCFLAGS) -o $@ $<

keypress.c:	keypress.def
	$(HSMP) $(HSMPFLAGS) -o $@ $<

keypress2:	keypress2.c
	$(CC) $(CCFLAGS) -o $@ $<

keypress2.c:	keypress2.def
	$(HSMP) $(HSMPFLAGS) -o $@ $<

keypress3:	keypress3.c
	$(CC) $(CCFLAGS) $(LDFLAGS) -o $@ $<

keypress3.c:	keypress3.def
	$(HSMP) $(HSMPFLAGS) -o $@ $<

clean:
	rm -f keypress.c keypress.h keypress.output keypress
	rm -f keypress2.c keypress2.h keypress2.output keypress2
	rm -f keypress3.c keypress3.h keypress3.output keypress3
	rm -fr keypress.dSYM keypress2.dSYM keypress3.dSYM


HSMP		= "$(shell git rev-parse --show-toplevel)"/src/hsmp
HSMPFLAGS	= -i -v -d -S -n -W 0
CC		= gcc
CCFLAGS		= -g -Wall

all: regions

regions:	regions.c
	$(CC) $(CCFLAGS) -o $@ $<

regions.c:	regions.def
	$(HSMP) $(HSMPFLAGS) -o $@ $<

clean:
	rm -f regions.c regions.h regions.output regions
	rm -fr regions.dSYM

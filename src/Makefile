CC	= gcc
CFLAGS	= -g -c -Wall
LDFLAGS	=
BISON	= bison
YFILE	= parser.y
YFLAGS	= -d -v
HFILES	= $(YFILE:.y=.h) lex.h defines.h gen.h license.h util.h
CFILES	= main.c $(YFILE:.y=.c) lex.c defines.c gen_c.c license.c util.c
CFILES2	= tool.c util.c
OBJS	= $(CFILES:.c=.o)
OBJS2	= $(CFILES2:.c=.o)
HSMP	= hsmp
TOOL	= tool

all: $(CFILES) $(HFILES) $(HSMP) $(TOOL)

$(HSMP): $(OBJS)
        $(CC) $(LDFLAGS) $(OBJS) -o $@

$(TOOL): $(OBJS2)
        $(CC) $(LDFLAGS) $(OBJS2) -o $@

.c.o:   
        $(CC) $(CFLAGS) $< -o $@

.y.h .y.c:
        $(BISON) $(YFLAGS) $< -o $@

clean:  
        rm -f $(OBJS) parser.c parser.h parser.output
        rm -f $(HSMP)
        rm -f $(OBJS2) $(TOOL)

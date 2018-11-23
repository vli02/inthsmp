
HSMP		= "$(shell git rev-parse --show-toplevel)"/src/hsmp
HSMPFLAGS	= -i -v -l py

all: keypress

keypress: keypress.def
	expand $< > $<.i
	$(HSMP) $(HSMPFLAGS) -o $@.py $<.i

clean:
	rm -f keypress.py keypress.def.i keypress.output keypress
	rm -f keypress2.py keypress2.def.i keypress2.output keypress2
	rm -f keypress3.py keypress3.def.i keypress3.output keypress3
	rm -fr keypress.dSYM keypress2.dSYM keypress3.dSYM

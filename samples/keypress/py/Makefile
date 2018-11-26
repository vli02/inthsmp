
HSMP		= "$(shell git rev-parse --show-toplevel)"/src/hsmp
HSMPFLAGS	= -i -v -l py

all: keypress keypress2

keypress: keypress.def
	expand $< > $<.i
	$(HSMP) $(HSMPFLAGS) -o $@.py $<.i

keypress2: keypress2.def
	expand $< > $<.i
	$(HSMP) $(HSMPFLAGS) -o $@.py $<.i

clean:
	rm -f keypress.py keypress.def.i keypress.output
	rm -f keypress2.py keypress2.def.i keypress2.output
	rm -f keypress3.py keypress3.def.i keypress3.output

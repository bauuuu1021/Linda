CFLAGS = -g -fopenmp

all: linda parser
FILE = linda parser

%: %.c
	g++ $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	$(RM) *.txt $(FILE)

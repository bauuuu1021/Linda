CFLAGS = -g -fopenmp

FILE = linda parser
all: $(FILE)

%: %.c
	g++ $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	$(RM) *.txt $(FILE)

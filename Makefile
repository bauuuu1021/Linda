CFLAGS = -g -fopenmp

FILE = linda parser
all: $(FILE)

%: %.cpp
	g++ $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	$(RM) *.txt $(FILE)

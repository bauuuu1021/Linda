CFLAGS = -g -fopenmp

FILE = 0856101
all: $(FILE)

%: %.cpp
	g++ $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	$(RM) *.txt $(FILE)

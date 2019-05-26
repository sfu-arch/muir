CC=clang
SOURCES=$(shell ls test*.c)
SCALA=$(SOURCES:%.c=%.scala)
DOT=$(shell ls *.dot)
SVG=$(DOT:%.dot=%.svg)
LDFLAGS=-lpthread


.PHONY: test_sim

test_sim: add_example_sim.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

fpga: add_example_fpga.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	-rm -f test_sim fpga *.bc *.ll *.dot *.scala *~ *.json

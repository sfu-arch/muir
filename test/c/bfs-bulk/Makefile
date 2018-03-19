KERN=bfs
ALG=bulk

CC=clang

CFLAGS?=-O3 -Wall -Wno-unused-label

CLANGFLAGS?=-O1 -Icommon

SRCS=$(KERN).c local_support.c common/support.c
FILES=$(SRCS) $(KERN).h common/support.h

$(KERN): $(FILES) common/harness.c
	$(CC) $(CFLAGS) -Icommon -o $(KERN) $(SRCS) common/harness.c

run: $(KERN) input.data check.data
	./$(KERN) input.data check.data

generate: $(FILES) generate.c
	$(CC) $(CFLAGS) -Icommon -o generate $(SRCS) generate.c
	./generate

bitcode: $(KERN).c
	$(CC) -emit-llvm -O1 -Icommon -S $< -o $@.bc


hls: $(KERN).c $(KERN).h
	vivado_hls hls.tcl

clean:
	rm -f $(KERN) generate output.data *.bc

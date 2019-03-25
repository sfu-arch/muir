KERN=fft
ALG=strided

<<<<<<< HEAD
CFLAGS?=-O1 -g -Wall -Wno-unused-label
=======
CFLAGS?=-O1 -Wall -Wno-unused-label
>>>>>>> dbe7b8c88f89e0f94435e47e73094c562c6353ed

SRCS=$(KERN).c local_support.c ../../common/support.c
FILES=$(SRCS) $(KERN).h ../../common/support.h

$(KERN): $(FILES) ../../common/harness.c
	$(CC) $(CFLAGS) -I../../common -o $(KERN) $(SRCS) ../../common/harness.c -lm

run: $(KERN) input.data check.data
	./$(KERN) input.data check.data

generate: $(FILES) generate.c
	$(CC) $(CFLAGS) -I../../common -o generate $(SRCS) generate.c -lm
	./generate

hls: $(KERN).c $(KERN).h
	vivado_hls hls.tcl

clean:
	rm -f $(KERN) generate output.data

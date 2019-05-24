KERN=gemm
ALG=blocked

CFLAGS?=-O1 -g -Wall -Wno-unused-label -fno-unroll-loops -fno-vectorize -fno-slp-vectorize

SRCS=$(KERN).c local_support.c ../../common/support.c
FILES=$(SRCS) $(KERN).h ../../common/support.h

DANDELION=$(HOME)/git/dandelion-generator/build/bin/dandelion
CONFIG=$(HOME)/git/dandelion-generator/scripts/config.json


$(KERN).scala : $(KERN).bc
	$(DANDELION) -fn-name=$(KERN) -config=$(CONFIG) $< -o $(KERN)

$(KERN).bc : $(KERN)
	extract-bc $<
	opt $@ -mem2reg -loop-simplify -loop-simplifycfg -simplifycfg -disable-loop-vectorization -dce -dot-cfg -o $@

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

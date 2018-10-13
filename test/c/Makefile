CC=clang
XKETCH=../../build/bin/dandelion
CFLAGS=-O0 -fno-unroll-loops -fno-vectorize -fno-slp-vectorize -emit-llvm -m32 -S -o -
SOURCES=$(shell ls test*.c)
SCALA=$(SOURCES:%.c=%.scala)
DOT=$(shell ls *.dot)
SVG=$(DOT:%.dot=%.svg)

.PRECIOUS: %.ll

all: $(SCALA)

svg: $(SVG)

%.bc: %.c
	$(CC) $(CFLAGS) $< -o - | opt -mem2reg -loop-simplify -loop-simplifycfg -disable-loop-vectorization -dce -dot-cfg -o $@

%.ll: %.bc
	llvm-dis $<

%.scala: %.ll
	$(XKETCH) -fn-name=$* -config=../../scripts/config.json $< -o $*

%.svg: %.dot
	dot -Tsvg $< -o $@

clean:
	-rm -f *.bc *.ll *.dot *.scala *~ *.json

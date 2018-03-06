CC=clang
XKETCH=../../build/bin/xketch
CFLAGS= -emit-llvm -m32 -S -o -
SOURCES=$(shell ls test*.c)
SCALA=$(SOURCES:%.c=%.scala)
DOT=$(shell ls *.dot)
SVG=$(DOT:%.dot=%.svg)

.PRECIOUS: %.ll

all: $(SCALA)

svg: $(SVG)

%.bc: %.c
	$(CC) $(CFLAGS) $< -o - | opt -dce -mem2reg -dot-cfg -o $@
	$(CC) -g $(CFLAGS) $< -o - | opt -dce -mem2reg -o $@

%.ll: %.bc
	llvm-dis $<

%.scala: %.ll
	$(XKETCH) -fn-name=$* $< -o $*

%.svg: %.dot
	dot -Tsvg $< -o $@

clean:
	rm -f *.bc *.ll *.dot *.scala *.svg *~

CC=clang
XKETCH=../../build/bin/xketch
CFLAGS=-g -emit-llvm -m32 -S -o -
SOURCES=$(shell ls test*.c)
SCALA=$(SOURCES:%.c=%.scala)

.PRECIOUS: %.ll

all: $(SCALA)

%.bc: %.c
	$(CC) $(CFLAGS) $< -o - | opt -mem2reg -dot-cfg -o $@

%.ll: %.bc
	llvm-dis $<

%.scala: %.ll
	$(XKETCH) -fn-name=$* $< -o $*
#	mv -f dfg.$*.dot $*.dot 2>/dev/null; true

%.svg: %.dot
	dot -Tsvg $< -o $@

clean:
	rm -f *.bc *.ll *.dot *.scala *.svg *~

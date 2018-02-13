CC=clang
XKETCH=../../../build/bin/xketch
CFLAGS= -DPIR -emit-llvm -m32 -S -o -
SOURCES=$(shell ls *.c)
SCALA=$(SOURCES:%.c=%.scala)

.PRECIOUS: %.ll

all: $(SCALA)

%.bc: %.c
	$(CC) $(CFLAGS) $< -o - | opt -mem2reg -dot-cfg -o $@
	$(CC) -g $(CFLAGS) $< -o - | opt -mem2reg -o $@

%.ll: %.bc
	llvm-dis $<

%.scala: %.ll
	$(XKETCH) -fn-name=$* $< -o $*

%.svg: %.dot
	dot -Tsvg $< -o $@

clean:
	rm -f *.bc *.ll *.dot *.scala *.svg *~

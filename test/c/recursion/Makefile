CC=clang
XKETCH=../../../build/bin/xketch
CFLAGS= -DPIR -g -fno-inline -emit-llvm -m32 -S -o -
OPTFLAGS= -disable-inlining -mem2reg -dot-cfg
SOURCES=$(shell ls *.c)
SCALA=$(SOURCES:%.c=%.scala)

.PRECIOUS: %.ll

all: $(SCALA)

%.bc: %.c
	$(CC) $(CFLAGS) $< -o - | opt $(OPTFLAGS) -o $@

%.ll: %.bc
	llvm-dis $<

%.scala: %.ll
	$(XKETCH) -fn-name=$* $< -o $*

%.svg: %.dot
	dot -Tsvg $< -o $@

clean:
	rm -f *.bc *.ll *.dot *.scala *~

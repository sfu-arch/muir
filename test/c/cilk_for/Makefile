CC=clang
CFLAGS=-g -emit-llvm -m32 -S -o -

all: trivial_for.ll int_exp.ll

%.bc: %.c
	$(CC) $(CFLAGS) $< -o - | opt -mem2reg -dot-cfg -o $@

%.ll: %.bc
	llvm-dis $<

%.scala: %.ll
	../../../build/bin/xketch -fn-name=$* $< -o $*
	mv dfg.$*.dot $*.dot

%.svg: %.dot
	dot -Tsvg $< -o $@

clean:
	rm -f *.bc *.ll *.dot *~

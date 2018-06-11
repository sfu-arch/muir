CC=clang
XKETCH=../../../build/bin/xketch
CFLAGS= -DPIR -emit-llvm -m32 -fno-vectorize -S -o -
SOURCES=$(shell ls *.c)
SCALA=$(SOURCES:%.c=%.scala)
DOT=$(shell ls *.dot)
SVG=$(DOT:%.dot=%.svg)
APALA=~/amoeba/apala-generator/build/bin/xketch

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

%.split: %.bc
	$(APALA) -fn-name=$* $< -o $*
	llvm-dis final.bc
	opt final.ll -S -dce -mem2reg -dot-cfg -o $*a.ll
	../../../build/bin/xketch -fn-name=$* $*a.ll -o $*
	../../../build/bin/xketch -fn-name=$*_detach1 $*a.ll -o $*_detach1

# Notes
# 
# ~/amoeba/apala-generator/build/bin/xketch -fn-name=cilk_for_test11 cilk_for_test11.bc -o cilk_for_test11
# llvm-dis final.bc
# opt final.ll -S -dce -mem2reg -dot-cfg -o cilk_for_test11a.ll
# May have to add "noinline" to functions in ll file 
# ../../../build/bin/xketch -fn-name=cilk_for_test11 cilk_for_test11a.ll -o cilk_for_test11
# ../../../build/bin/xketch -fn-name=cilk_for_test11_detach1 cilk_for_test11a.ll -o cilk_for_test11_detach
# make svg

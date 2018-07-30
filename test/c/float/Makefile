CC=clang
XKETCH=../../../build/bin/dandelion
CFLAGS= -DPIR -DUSE_CILK_API -emit-llvm -m32 -S 
SOURCES=$(shell ls *.c)
SCALA=$(SOURCES:%.c=%.scala)
DOT=$(shell ls *.dot)
SVG=$(DOT:%.dot=%.svg)
#APALA=~/amoeba/apala-generator/build/bin/xketch
APALA=~/git/xketch-ap/build/bin/xketch
#APALA=/home/amiralis/git/xketch-ap/build/bin/xketch

.PRECIOUS: %.ll

all: $(SCALA)

svg: $(SVG)

%.bc: %.c
	$(CC) $(CFLAGS) $< -o - | opt -dce -mem2reg -dot-cfg -o $@

%.ll: %.bc
	llvm-dis $<

%.scala: %.ll
	$(XKETCH) -fn-name=$* $< -o $*

%.svg: %.dot
	dot -Tsvg $< -o $@

clean:
	-rm -f *.bc *.ll *.dot *.scala *.svg *~ *.json

%.split: %.bc
	$(APALA) -fn-name=$* $< -o $*
	llvm-dis final.bc
	opt final.ll -S -dce -mem2reg -dot-cfg -o $*a.ll
	$(XKETCH) -fn-name=$* $*a.ll -o $*
	$(eval subfcn = $(shell egrep -o "$*_detach[0-9]*" $*a.ll | sort -u))
	for fcn in ${subfcn} ; do \
	   $(XKETCH) -fn-name=$${fcn} $*a.ll -o $${fcn} ;  \
	done



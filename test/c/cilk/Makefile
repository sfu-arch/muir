CC=clang
XKETCH=../../../build/bin/dandelion
SPLIT=../../../build/bin/tapir-extract
CFLAGS=-DPIR -DUSE_CILK_API -emit-llvm -m32 -S -o -
SOURCES=$(shell ls *.c)
SCALA=$(SOURCES:%.c=%.scala)
DETACH=$(SOURCES:%.c=%.detach)
DOT=$(shell ls *.dot)
SVG=$(DOT:%.dot=%.svg)
#APALA=~/amoeba/apala-generator/build/bin/xketch

.PRECIOUS: %.ll

all: $(SCALA)

svg: $(SVG)

%.bc: %.c
	$(CC) $(CFLAGS) $< -o - | opt -separate-const-offset-from-gep -dce -mem2reg -dot-cfg -o $@

%.ll: %.bc
	llvm-dis $<

%.scala: %.ll
	$(XKETCH) -fn-name=$* -config=../../../scripts/config.json $< -o $*

%.svg: %.dot
	dot -Tsvg $< -o $@

clean:
	-rm -f *.bc *.ll *.dot *.scala *.svg *~ *.json


.ONESHELL:
%.detach: %.bc
	$(SPLIT) -fn-name=$* $< -o $*
	llvm-dis final.bc
	opt final.ll -S -dce -separate-const-offset-from-gep -mem2reg -dot-cfg -o $*a.ll
	$(XKETCH) -fn-name=$* -config=../../../scripts/config.json $*a.ll -o $*
	$(eval subfcn=$(shell egrep -o "$*_detach[0-9]*" $*a.ll | sort -u))
	for fcn in ${subfcn} ; do \
		$(XKETCH) -fn-name=$${fcn} -config=../../../scripts/config.json $*a.ll -o $${fcn} ;  \
	done

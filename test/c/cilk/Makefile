CC=clang
XKETCH=../../../build/bin/dandelion
SPLIT=../../../build/bin/tapir-extract
CFLAGS=-O1 -DPIR -DUSE_CILK_API -g -emit-llvm -m32 -S -o -
SOURCES=$(shell ls *.c)
SCALA=$(SOURCES:%.c=%.scala)
DETACH=$(SOURCES:%.c=%.detach)
DOT=$(shell ls *.dot)
SVG=$(DOT:%.dot=%.svg)
EXTRACT=../../../build/bin/tapir-extract

.PRECIOUS: %.ll

all: $(SCALA)

svg: $(SVG)

%.bc: %.c
	$(CC) $(CFLAGS) $< -o - | opt -dce -mem2reg -dot-cfg -o $@

%.ll: %.bc
	llvm-dis $<

%.scala: %.detach
	$(XKETCH) -fn-name=$* -config=../../../scripts/config.json $*_detach.bc -o $*

%.svg: %.dot
	dot -Tsvg $< -o $@

clean:
	-rm -f *.bc *.ll *.dot *.scala *.svg *~ *.json


.ONESHELL:
%.detach: %.bc
	$(SPLIT) -fn-name=$* $< -o $*

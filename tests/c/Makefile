LLVM_DIR=/home/amirali/git/Tapir-Meta/tapir/build/bin
CC=$(LLVM_DIR)/clang
OPT=$(LLVM_DIR)/opt
DIS=$(LLVM_DIR)/llvm-dis

ifeq (, $(shell which clang))
$(error "There is no clang compiler in PATH, consider source installing dandelion dependencies and setting up your PATH")
endif

ifeq (, $(shell which dandelion))
$(error "There is no dandelion in PATH, consider source scripts/setup.sh")
endif

DANDELION=dandelion
CFLAGS=-O1 -g -fno-unroll-loops -fno-vectorize -fno-slp-vectorize -emit-llvm -S -o -
SOURCES=$(shell ls *.c)
SCALA=$(SOURCES:%.c=%.scala)
DOT=$(shell ls *.dot)
SVG=$(DOT:%.dot=%.svg)
DANDELION_CONFIG=../../scripts/config.json

.PRECIOUS: %.ll

all: $(SCALA)

svg: $(SVG)

%.bc: %.c
	$(CC) $(CFLAGS) $< -o - | $(OPT) -mem2reg -loop-simplify -loop-simplifycfg -simplifycfg -disable-loop-vectorization -dce -dot-cfg -o $@

%.ll: %.bc
	$(DIS) $<

%.scala: %.ll
	$(DANDELION) -fn-name=$* -config=$(DANDELION_CONFIG) $< -o $*

%.svg: %.dot
	dot -Tsvg $< -o $@

clean:
	-rm -f *.bc *.ll *.dot *.scala *~ *.json

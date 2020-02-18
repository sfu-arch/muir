CC=clang

ifeq (, $(shell which clang))
$(error "There is no clang compiler in PATH, consider source installing dandelion dependencies and setting up your PATH")
endif

ifeq (, $(shell which dandelion))
$(error "There is no dandelion in PATH, consider source scripts/setup.sh")
endif

DANDELION=dandelion
CFLAGS=-O1 -g -fno-unroll-loops -fno-vectorize -fno-slp-vectorize -emit-llvm -S -o -
SOURCES=$(shell ls test*.c)
SOURCES := vector_scale.c conv2dSerial.c
SCALA=$(SOURCES:%.c=%.scala)
DOT=$(shell ls *.dot)
SVG=$(DOT:%.dot=%.svg)
DANDELION_CONFIG=@CMAKE_CURRENT_BINARY_DIR@/scripts/config.json

.PRECIOUS: %.ll

all: $(SCALA)

svg: $(SVG)

%.bc: %.c
	$(CC) $(CFLAGS) $< -o - | opt -mem2reg -loop-simplify -loop-simplifycfg -simplifycfg -disable-loop-vectorization -dce -dot-cfg -o $@

%.ll: %.bc
	llvm-dis $<

%.scala: %.ll
	$(DANDELION) -fn-name=$* -config=$(DANDELION_CONFIG) $< -o $*

%.svg: %.dot
	dot -Tsvg $< -o $@

clean:
	-rm -f *.bc *.ll *.dot *.scala *~ *.json

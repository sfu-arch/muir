include ../../common.mk
KERN=fft
ALG=strided

ifeq (, $(shell which clang))
$(error "There is no clang compiler in PATH, consider source installing dandelion dependencies and setting up your PATH")
endif

ifeq (, $(shell which dandelion))
$(error "There is no dandelion in PATH, consider source scripts/setup.sh")
endif

BC=$(shell ls *.bc)
SCALA=$(BC:%.bc=%.scala)

.PRECIOUS: %.bc

all: dandelion

dandelion: $(KERN).bc
	$(DANDELION) -fn-name=$(KERN) -config=$(DANDELION_CONFIG) $< -o $(KERN)

clean:
	-rm -f *.final.bc *.ll *.scala *~ *.json

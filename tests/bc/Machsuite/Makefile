BENCHMARKS=\
	fft/strided \
	gemm \
	spmv/crs \
	stencil/stencil2d \
	stencil/stencil3d \
#FIXME\
	backprop/backprop \

.PHONY: dandelion

dandelion:
	@( for b in $(BENCHMARKS); do $(MAKE) -C $$b dandelion; done )


### For regression tests
all: dandelion

clean:
	@( for b in $(BENCHMARKS); do $(MAKE) -C $$b clean || exit ; done )

ifndef OPENRNG_ROOT
$(error OPENRNG_ROOT not set, rerun with make OPENRNG_ROOT=/path/to/openrng/install)
endif

CFLAGS=-O3 -I${OPENRNG_ROOT}/include
CXXFLAGS=${CFLAGS}
LDFLAGS=-L${OPENRNG_ROOT}/lib -L${OPENRNG_ROOT}/lib64 -lopenrng -lm

all: pi skipahead sobol_joe_kuo

pi: pi.c
	${CC} $^ -o $@ ${CFLAGS} ${LDFLAGS}

skipahead: skipahead.c
	${CC} $^ -o $@ ${CFLAGS} ${LDFLAGS} -fopenmp

sobol_joe_kuo: sobol_joe_kuo.cpp
	${CXX} $^ -o $@ ${CXXFLAGS} ${LDFLAGS}

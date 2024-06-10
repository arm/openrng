ifndef OPENRNG_ROOT
$(error OPENRNG_ROOT not set, rerun with make OPENRNG_ROOT=/path/to/openrng/install)
endif

CFLAGS=-O3 -I${OPENRNG_ROOT}/include
LDFLAGS=-L${OPENRNG_ROOT}/lib -lopenrng -lm

all: pi skipahead

pi: pi.c
	${CC} $^ -o $@ ${CFLAGS} ${LDFLAGS}

skipahead: skipahead.c
	${CC} $^ -o $@ ${CFLAGS} ${LDFLAGS} -fopenmp

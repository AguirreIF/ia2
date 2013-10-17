CC       := cc
CFLAGS   := -O3 -g -Wall -Wextra -pedantic -std=gnu99 -march=native -fgnu89-inline \
			-fexpensive-optimizations -funroll-loops \
			-fmove-loop-invariants -fprefetch-loop-arrays -ftree-loop-optimize \
			-ftree-vect-loop-version -ftree-vectorize
LDFLAGS  := -lm -lgmp -lrt

SRC := programa.c funciones.c entrada.tab.c entrada.c shunt.c mtwist.c
OBJ := $(SRC:.c=.o)

all: programa

programa: ${OBJ}
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

programa.o: programa.c entrada.tab.c
	${CC} ${CFLAGS} -c $^

entrada.c: entrada.l entrada.tab.h
	flex $<

%.tab.c %.tab.h: %.y
	bison $<

clean:
	rm -f ${OBJ} programa entrada.tab.c entrada.tab.h entrada.c

.PHONY: all clean

CC       := cc
CFLAGS   := -g -Wall -Wextra -pedantic -std=gnu99
LDFLAGS  := -lm -lgmp

SRC := programa.c funciones.c entrada.tab.c entrada.c shunt.c
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

CC := gcc
SRC := buf.c
OUT := piecetable
OPTS := -g -Wall -Wpedantic

all: $(static)

static:
	${CC} $(OPTS) -c $(SRC) -o $(OUT).o
	ar rcs lib$(OUT).a $(OUT).o
test:
	${CC} $(OPTS) $(SRC) test.c -o $(OUT)-test

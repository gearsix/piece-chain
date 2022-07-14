CC := gcc
SRC := buffer.c
OUT := ec
OPTS := -g -Wall -Wpedantic -std=c89

all:
	${CC} $(OPTS) $(SRC) main.c -o $(OUT)
static:
	${CC} $(OPTS) -c $(SRC) -o $(OUT).o
	ar rcs lib$(OUT).a $(OUT).o
test:
	${CC} $(OPTS) $(SRC) test*c -o $(OUT)-test

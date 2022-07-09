CC := gcc
SRC := buffer.c
OUT := ec
OPTS := -g -Wall -Wpedantic -std=c89

all:
	${CC} $(OPTS) $(SRC) main.c -o $(OUT)
test:
	${CC} $(OPTS) $(SRC) test*c -o $(OUT)-test

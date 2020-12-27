CC=g++
CFLAGS=-std=c++11 -g -W -Wall -Wextra
LDFLAGS=-g
EXEC=SAkuna

all: $(EXEC)

SAkuna: main.o sakuna.o board.o magicmoves.o piece.o move.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)

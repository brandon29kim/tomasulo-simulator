CC = gcc 
CFLAGS = -g
DEBUGFLAGS = -DDEBUG

sim: main.o fu.o tomasulo.o output.o
	 $(CC) $(CFLAFS) -o sim main.o fu.o tomasulo.o output.o

debug: main_debug.o fu.o tomasulo.o output.o
	$(CC) $(CFLAGS) $(DEBUGFLAGS) -o sim main_debug.o fu.o tomasulo.o output.o

asm: asm.o
	$(CC) $(CFLAGS) -o asm asm.c

main_debug.o: main.c fu.h tomasulo.h output.h
	$(CC) $(CFLAGS) $(DEBUGFLAGS) -c -o main_debug.o main.c

main.o: main.c fu.h tomasulo.h output.h

fu.o: fu.c fu.h tomasulo.h

tomasulo.o: tomasulo.c fu.h tomasulo.h

output.o: output.c fu.h tomasulo.h output.h

asm.o: asm.c

clean:
	rm -f sim main.o main_debug.o fu.o tomasulo.o output.o asm asm.o

all: chip8.exe

chip8.exe: chip8.o
	gcc -o chip8.exe chip8.o

chip8.o: chip8.c
	gcc -c chip8.c

clean:
	rm chip8.o chip8.exe
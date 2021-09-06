chip8:
	gcc -Lsrc/lib -Isrc/include -o chip8 src/chip8.c -std=c11 -Wall  -lmingw32 -lSDL2main -lSDL2


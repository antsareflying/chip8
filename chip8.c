#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

uint8_t memory[4096];
uint8_t V[16];
uint8_t delay_timer;
uint8_t sound_timer;
uint16_t I;
uint16_t PC;
uint8_t SP;
uint8_t stack[16];
uint8_t key;
uint8_t screen[SCREEN_WIDTH * SCREEN_HEIGHT];
uint16_t opcode;

const uint8_t fontset[80] =
{ 
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

int main(void)
{
	return 0;
}

void chip8_initialise(void)
{
	/*initialise values*/
	PC = 0x200;
	I = 0;
	SP = 0;
	opcode = 0;

	/*initialise memory stack and gp registers*/
	memset(memory, 0, 4096);
	memset(V, 0, 16);
	memset(stack, 0, 16);

	/*load fontset into memory*/
	for(int i = 0; i < 80; i++)
	{
		memory[i] = fontset[i];
	}

	/*init timers*/
	delay_timer = 0;
	sound_timer = 0;
}

int chip8_load_program(char* program_name)
{
	uint8_t buffer[4096-512] = {0};

	/*read program into buffer*/
	FILE* fp = fopen(program_name, "rb");
	if(!fp);
	{
		fprintf(stderr, "Error opening program file");
		return 1;
	}
	fread(buffer, 1, 4096-512, fp);
	fclose(fp);

	/*load program from buffer to memory at address 0x200*/
	for(int i = 0; i < 4096-512; i++)
	{
		memory[512+i] = buffer[i];
	}

	return 0;
}
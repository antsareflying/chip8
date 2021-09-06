#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define unknown_opcode(opcode)\
		do { \
			fprintf(stderr, "Unknown opcode: %x", opcode); \
			exit(EXIT_FAILURE); \
		} while (0)


void chip8_initialise(void);
int chip8_load_program(char* program_name);
void fetch_decode_execute(void);
void chip8_cleanup(void);

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

int main(int argc, char** argv)
{
	chip8_initialise();
	SDL_Delay(10000);
	chip8_cleanup();
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

	/*start SDL*/
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window = SDL_CreateWindow("Chip-8",
											SDL_WINDOWPOS_CENTERED,
											SDL_WINDOWPOS_CENTERED, 10 * SCREEN_WIDTH,
											10 * SCREEN_HEIGHT,
											0);
	SDL_Renderer* renderer = SDL_CreateRenderer(window,
												-1,
												0);
	SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
	SDL_RenderClear(renderer);
	
}

int chip8_load_program(char* program_name)
{
	uint8_t buffer[4096-512] = {0};

	/*read program into buffer*/
	FILE* fp = fopen(program_name, "rb");
	if(!fp)
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

void chip8_fetch_decode_execute(void)
{
	/*fetch opcode*/
	opcode = memory[PC] << 8 | memory[PC + 1];
	uint8_t X = (opcode >> 8) & 0x000F;
	uint8_t Y = (opcode >> 4) & 0x000F;
	uint8_t random_no = (rand() % 256); /*generates rand number between 0 and 255*/
	
	/*decode and execute opcode*/
	switch(opcode & 0xF000)
	{
		case 0x0000:
			break;
		case 0x1000: /*1NNN jumps to address NNN*/
			PC = opcode & 0xFFF;
			break;
		case 0x2000:
			break;
		case 0x3000: /*3XNN Skips the next instruction if VX equals NN*/
			PC += (V[X] == (opcode & 0x00FF)) ? 4 : 2;
			break;
		case 0x4000: /*4XNN Skips the next instruction if VX doesn't equal NN*/
			PC += (V[X] != (opcode & 0x00FF)) ? 4 : 2;
			break;
		case 0x5000: /*5XY0 Skips the next instruction if VX equals VY*/
			switch(opcode & 0x000F)
			{
				case 0:
					PC += (V[X] == V[Y]) ? 4 : 2;
					break;
				default:
					unknown_opcode(opcode);
			}
			break;
		case 0x6000: /*6XNN sets VX to NN*/
			V[X] = opcode & 0x00FF;
			PC += 2;
			break;
		case 0x7000: /* 7XNN Adds NN to VX*/
			V[X] += opcode & 0x00FF;
			PC += 2;
			break;
		case 0x8000: /*arthmetic*/
			switch(opcode & 0x000F)
			{
				case 0x0000:
					V[X] = V[Y];
					break;
				case 0x0001:
					V[X] = V[X] | V[Y];
					break;
				case 0x0002:
					V[X] = V[X] & V[Y];
					break;
				case 0x0003:
					V[X] = V[X] ^ V[Y];
					break;
				case 0x0004:
					V[0xF] = (V[Y] > 0xFF - V[X]) ? 1 : 0;
					V[X] += V[Y];
					break;
				case 0x0005:
					V[0xF] = (V[Y] > V[X]) ? 1 : 0;
					V[X] -= V[Y];
					break;
				case 0x0006:
					V[0xF] = V[X] & 1;
					V[X] >>= 1;
					break;
				case 0x0007:
					V[0xF] = (V[X] > V[Y]) ? 0 : 1;
					V[X] = V[Y] - V[X];
					break;
				case 0x000E:
					V[0xF] = V[X] & 1;
					V[X] <<= 1;
					break;
				default:
					unknown_opcode(opcode);
			}
			PC += 2;
			break;
		case 0x9000:
			switch(opcode & 0x000F)
				{
					case 0:
						PC += (V[X] != V[Y]) ? 4 : 2;
						break;
					default:
						unknown_opcode(opcode);
				}
			break;
		case 0xA000: /*ANNN sets I to address NNN*/
			I = opcode & 0x0FFF;
			PC += 2;
			break;
		case 0xB000: /*BNNN jumps to address NNN plus V0*/
			PC = (opcode & 0x0FFF) + V[0];
			break;
		case 0xC000: /*CXNN sets VX to result of & on rand no and NN*/
			V[X] = random_no & (opcode & 0x00FF);
			PC += 2;
			break;
		case 0xD000:
			break;
		case 0xE000:
			break;
		case 0xF000:
			break;
		default:
			unknown_opcode(opcode);
	}
}

void chip8_cleanup(void)
{
	SDL_Quit();
}
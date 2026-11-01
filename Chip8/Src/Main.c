#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include <conio.h>
#include <stdlib.h>
#include <Windows.h>
#include <Graphics.h>

/*
The stack is an array of 16 16-bit values, used to store the address that the
interpreter shoud return to when finished with a subroutine.
Chip-8 allows for up to 16 levels of nested subroutines.
*/

#define MEM_SIZE 4096
#define USER_MEM_START 0x200
#define USER_MEM_SIZE (MEM_SIZE - USER_MEM_START)
#define FONTSET_START 0x50
#define STACK_SIZE 16

const uint8_t fontset[80] = {
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

static const int key_map[16] = {
	'X', '1', '2', '3',   // 0 1 2 3
	'Q', 'W', 'E', 'A',   // 4 5 6 7
	'S', 'D', 'Z', 'C',   // 8 9 A B
	'4', 'R', 'F', 'V'    // C D E F
};

typedef struct chip8 {
	uint8_t			v[16];
	uint16_t		i;
	uint16_t		pc;
	uint8_t			sp;

	uint8_t			delay;
	uint8_t			sound;
	uint8_t			screen_dirty;
	uint8_t			vblank;

	uint8_t			ram[MEM_SIZE];
	uint32_t		video[CHIP8_WIDTH * CHIP8_HEIGHT];
	uint16_t		stack[STACK_SIZE];
} chip8;

int
ch_key_down(uint8_t k) {
	return (GetAsyncKeyState(key_map[k & 0xF]) & 0x8000) != 0;
}

void
ch_reset(chip8* const ch) {
	memset(ch, 0x00, sizeof(chip8));

	ch->pc = 0x200;
	ch->screen_dirty = 1;
}

void
ch_load_fontset(chip8* const ch) {
	for (uint32_t i = 0; i < 80; ++i) {
		ch->ram[FONTSET_START + i] = fontset[i];
	}
}

int
ch_load_from_file(chip8* const ch, const char* file) {
	if (!file)
		return -1;

	FILE* f = fopen(file, "rb");
	if (!f) {
		perror("fopen");
		return -1;
	}

	if (fseek(f, 0, SEEK_END) != 0) {
		fclose(f);
		return -1;
	}

	long size = ftell(f);
	if (!size || size > USER_MEM_SIZE) {
		fprintf(stderr, "File is empty or exceeds memory size!");
		fclose(f);
		return -1;
	}

	rewind(f);

	size_t read = fread(
		(void*)&ch->ram[USER_MEM_START],
		1,
		size,
		f);
	fclose(f);

	if (read < size) {
		fprintf(stderr, "Short read\n");
		return -1;
	}

	return 0;
}

uint16_t
ch_load16(chip8* const ch, uint16_t addr) {
	if (addr >= 0xFFF) {
		return 0;
	}

	const uint8_t ms = ch->ram[addr];
	const uint8_t ls = ch->ram[addr + 1];
	return (uint16_t)(ms << 8 | ls);
}

void
ch_print_instruction(chip8* const ch) {
	const uint16_t op = ch_load16(ch, ch->pc);
	printf("0x%04X OpCode: 0x%04X\n", ch->pc, op);
}

void
ch_00E0(chip8* const ch, uint16_t opcode) {//CLS
	memset(&ch->video[0], 0x00, CHIP8_WIDTH * CHIP8_HEIGHT * sizeof(uint32_t));
	ch->screen_dirty = 1;
}

void
ch_00EE(chip8* const ch, uint16_t opcode) {//RET
	if (ch->sp == 0) return;
	ch->pc = ch->stack[--ch->sp];
}

void
ch_1nnn(chip8* const ch, uint16_t opcode) {//JP addr
	ch->pc = (opcode & 0x0FFF);
}

void
ch_2nnn(chip8* const ch, uint16_t opcode) {//CALL addr
	ch->stack[ch->sp] = ch->pc;
	++ch->sp;
	ch->pc = (opcode & 0x0FFF);
}

void
ch_3xkk(chip8* const ch, uint16_t opcode) {//SE Vx, byte
	uint8_t vx = (opcode & 0x0F00) >> 8;
	if (ch->v[vx] == (opcode & 0xFF)) {
		ch->pc += 2;
	}
}

void
ch_4xkk(chip8* const ch, uint16_t opcode) {//SNE Vx, byte
	uint8_t vx = (opcode & 0x0F00) >> 8;
	if (ch->v[vx] != (opcode & 0xFF)) {
		ch->pc += 2;
	}
}

void
ch_5xy0(chip8* const ch, uint16_t opcode) {//SE Vx, Vy
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;
	if (ch->v[vx] == ch->v[vy]) {
		ch->pc += 2;
	}
}

void
ch_6xkk(chip8* const ch, uint16_t opcode) {//LD Vx, byte
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t byte = opcode & 0xFF;
	ch->v[vx] = byte;
}

void
ch_7xkk(chip8* const ch, uint16_t opcode) {//ADD Vx, byte
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t byte = opcode & 0xFF;
	ch->v[vx] += byte;
}

void
ch_8xy0(chip8* const ch, uint16_t opcode) {//LD Vx, Vy
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;
	ch->v[vx] = ch->v[vy];
}

void
ch_8xy1(chip8* const ch, uint16_t opcode) {//OR Vx, Vy
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;

	ch->v[0xF] = 0;
	ch->v[vx] |= ch->v[vy];
}

void
ch_8xy2(chip8* const ch, uint16_t opcode) {//AND Vx, Vy
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;

	ch->v[0xF] = 0;
	ch->v[vx] &= ch->v[vy];
}

void
ch_8xy3(chip8* const ch, uint16_t opcode) {//XOR Vx, Vy
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;

	ch->v[0xF] = 0;
	ch->v[vx] ^= ch->v[vy];
}

void
ch_8xy4(chip8* const ch, uint16_t opcode) {//ADD Vx, Vy
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;
	uint16_t sum = ch->v[vx] + ch->v[vy];

	ch->v[vx] = sum & 0xFF;
	ch->v[0xF] = sum > 0xFF;
}

void
ch_8xy5(chip8* const ch, uint16_t opcode) {//SUB Vx, Vy
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;
	uint8_t res = ch->v[vx] - ch->v[vy];
	uint8_t f = ch->v[vx] >= ch->v[vy];

	ch->v[vx] = res;
	ch->v[0xF] = f;
}

void
ch_8xy6(chip8* const ch, uint16_t opcode) {//SHR Vx {, Vy}
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;
	uint8_t src = ch->v[vy];

	ch->v[vx] = src >> 1;
	ch->v[0xF] = src & 1;
}

void
ch_8xy7(chip8* const ch, uint16_t opcode) {//SUBN Vx, Vy
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;

	ch->v[vx] = ch->v[vy] - ch->v[vx];
	ch->v[0xF] = (ch->v[vy] >= ch->v[vx]);
}

void
ch_8xyE(chip8* const ch, uint16_t opcode) {//SHL Vx {, Vy}
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;
	uint8_t src = ch->v[vy];
	
	ch->v[vx] = (uint8_t)(src << 1);
	ch->v[0xF] = (src & 0x80) >> 7;
}

void
ch_9xy0(chip8* const ch, uint16_t opcode) {//SNE Vx, Vy
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;

	if (ch->v[vx] != ch->v[vy]) {
		ch->pc += 2;
	}
}

void
ch_Annn(chip8* const ch, uint16_t opcode) {//LD I, addr
	uint16_t addr = opcode & 0x0FFF;
	ch->i = addr;
}

void
ch_Bnnn(chip8* const ch, uint16_t opcode) {//JP V0, addr
	uint16_t addr = opcode & 0x0FFF;
	ch->pc = addr + ch->v[0];
}

void
ch_Cxkk(chip8* const ch, uint16_t opcode) {//RND Vx, byte
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t byte = opcode & 0xFF;
	uint8_t rnd = rand() & 0xFF;

	ch->v[vx] = rnd & byte;
}

void
ch_Dxyn(chip8* const ch, uint16_t opcode) {//DRW Vx, Vy, nibble
	//if (!ch->vblank) { ch->pc -= 2; return; }
	ch->vblank = 0;

	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;
	uint8_t n = opcode & 0x000F;
	uint8_t px = ch->v[vx] % CHIP8_WIDTH;
	uint8_t py = ch->v[vy] % CHIP8_HEIGHT;

	ch->v[0xF] = 0;

	for (uint32_t i = 0; i < n; ++i) {
		uint32_t y = py + i;
		if (y >= CHIP8_HEIGHT) break;

		uint8_t byte = ch->ram[(ch->i + i) & 0x0FFF];

		for (uint32_t j = 0; j < 8; ++j) {
			uint32_t x = px + j;
			if (x >= CHIP8_WIDTH) break;

			if (byte & (0x80u >> j)) {
				uint32_t* pixel = &ch->video[y * CHIP8_WIDTH + x];
				if (*pixel) ch->v[0xF] = 1;
				*pixel ^= 1;
			}
		}
	}

	ch->screen_dirty = 1;
}

void
ch_Ex9E(chip8* const ch, uint16_t opcode) {//SKP Vx
	uint8_t vx = (opcode & 0x0F00) >> 8;

	if (ch_key_down(ch->v[vx])) ch->pc += 2;
}

void
ch_ExA1(chip8* const ch, uint16_t opcode) {//SKNP Vx
	uint8_t vx = (opcode & 0x0F00) >> 8;

	if (!ch_key_down(ch->v[vx])) ch->pc += 2;
}

void
ch_Fx07(chip8* const ch, uint16_t opcode) {//LD Vx, DT
	uint8_t vx = (opcode & 0x0F00) >> 8;
	ch->v[vx] = ch->delay;
}

void
ch_Fx0A(chip8* const ch, uint16_t opcode) {//LD Vx, K
	uint8_t vx = (opcode & 0x0F00) >> 8;

	for (uint8_t i = 0; i < 16; ++i) {
		if (ch_key_down(i)) {
			ch->v[vx] = i;
			while (ch_key_down(i)) {}
			return;
		}
	}

	ch->pc -= 2;
}

void
ch_Fx15(chip8* const ch, uint16_t opcode) {//LD DT, Vx
	uint8_t vx = (opcode & 0x0F00) >> 8;
	ch->delay = ch->v[vx];
}

void
ch_Fx18(chip8* const ch, uint16_t opcode) {//LD ST, Vx
	uint8_t vx = (opcode & 0x0F00) >> 8;
	ch->sound = ch->v[vx];
}

void
ch_Fx1E(chip8* const ch, uint16_t opcode) {//ADD I, Vx
	uint8_t vx = (opcode & 0x0F00) >> 8;
	ch->i += ch->v[vx];
}

void
ch_Fx29(chip8* const ch, uint16_t opcode) {//LD F, Vx
	uint8_t vx = (opcode & 0x0F00) >> 8;
	ch->i = FONTSET_START + ch->v[vx] * 5;
}

void
ch_Fx33(chip8* const ch, uint16_t opcode) {//LD B, Vx
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t val = ch->v[vx];

	ch->ram[ch->i + 2] = val % 10;
	val /= 10;
	ch->ram[ch->i + 1] = val % 10;
	val /= 10;
	ch->ram[ch->i] = val % 10;
}

void
ch_Fx55(chip8* const ch, uint16_t opcode) {//LD [I], Vx
	uint8_t vx = (opcode & 0x0F00) >> 8;
	for (uint32_t i = 0; i <= vx; ++i) {
		ch->ram[ch->i + i] = ch->v[i];
	}

	ch->i += vx + 1;
}

void
ch_Fx65(chip8* const ch, uint16_t opcode) {//LD Vx, [I]
	uint8_t vx = (opcode & 0x0F00) >> 8;
	for (uint32_t i = 0; i <= vx; ++i) {
		ch->v[i] = ch->ram[ch->i + i];
	}

	ch->i += vx + 1;
}

void
ch_cpu_step(chip8* const ch) {
	ch_print_instruction(ch);

	//get opcode
	uint16_t op = ch_load16(ch, ch->pc);

	//inc pc
	ch->pc += 2;

	//execute
	switch ((op & 0xF000) >> 12)
	{
	case 0x0: {
		if ((op & 0xF) == 0)			ch_00E0(ch, op);
		else if ((op & 0xF) == 0xE)		ch_00EE(ch, op);
	} break;
	case 0x1:							ch_1nnn(ch, op); break;
	case 0x2:							ch_2nnn(ch, op); break;
	case 0x3:							ch_3xkk(ch, op); break;
	case 0x4:							ch_4xkk(ch, op); break;
	case 0x5:							ch_5xy0(ch, op); break;
	case 0x6:							ch_6xkk(ch, op); break;
	case 0x7:							ch_7xkk(ch, op); break;
	case 0x8:
		switch (op & 0xF) {
		case 0x0:						ch_8xy0(ch, op); break;
		case 0x1:						ch_8xy1(ch, op); break;
		case 0x2:						ch_8xy2(ch, op); break;
		case 0x3:						ch_8xy3(ch, op); break;
		case 0x4:						ch_8xy4(ch, op); break;
		case 0x5:						ch_8xy5(ch, op); break;
		case 0x6:						ch_8xy6(ch, op); break;
		case 0x7:						ch_8xy7(ch, op); break;
		case 0xE:						ch_8xyE(ch, op); break;
		default: break;
		} break;
	case 0xA:							ch_Annn(ch, op); break;
	case 0xB:							ch_Bnnn(ch, op); break;
	case 0xC:							ch_Cxkk(ch, op); break;
	case 0xD:							ch_Dxyn(ch, op); break;
	case 0xE: {
		if ((op & 0xFF) == 0x9E)		ch_Ex9E(ch, op);
		else if ((op & 0xFF) == 0xA1)	ch_ExA1(ch, op);
	} break;
	case 0xF:
		switch (op & 0xFF)
		{
		case 0x07:						ch_Fx07(ch, op); break;
		case 0x0A:						ch_Fx0A(ch, op); break;
		case 0x15:						ch_Fx15(ch, op); break;
		case 0x18:						ch_Fx18(ch, op); break;
		case 0x1E:						ch_Fx1E(ch, op); break;
		case 0x29:						ch_Fx29(ch, op); break;
		case 0x33:						ch_Fx33(ch, op); break;
		case 0x55:						ch_Fx55(ch, op); break;
		case 0x65:						ch_Fx65(ch, op); break;
		default: break;
		} break;
	default: break;
	}
}

int main() {
	chip8 chip;

	ch_reset(&chip);
	ch_load_fontset(&chip);

	if (ch_load_from_file(&chip, "Content\\bin\\superpong.ch8") != 0) {
		return -1;
	}

	graphics* g = graphics_create("Chip8", 16);
	ULONGLONG time = GetTickCount64();

	while (graphics_window_open(g)) {
		if (GetAsyncKeyState(VK_SPACE)) {
			_getch();
		}

		ch_cpu_step(&chip);

		ULONGLONG now = GetTickCount64();
		if (now - time >= 16) {
			time = now;
			graphics_draw(g, &chip.video[0]);

			if (chip.delay > 0) {
				--chip.delay;
			}
			if (chip.sound > 0) {
				--chip.sound;
			}

			chip.vblank = 1;

			for (uint8_t i = 0; i < 16; ++i) {
				if (ch_key_down(i)) {
					printf("Key %X is down\n", i);
				}
			}
		}
	}

	graphics_destroy(g);

	return 0;
}

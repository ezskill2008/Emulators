#pragma once
#include <Common.h>
#include <Bus.h>
#include <Inst.h>

/*
16-bit	Hi	Lo	Name/Function
AF		A	-	Accumulator & Flags
BC		B	C	BC
DE		D	E	DE
HL		H	L	HL
SP		-	-	Stack Pointer
PC		-	-	Program Counter/Pointer

The Flags Register (lower 8 bits of AF register)
Bit	Name	Explanation
7	z		Zero flag
6	n		Subtraction flag (BCD)
5	h		Half Carry flag (BCD)
4	c		Carry flag
*/

#define CPU_START_PC 0x0100
#define CPU_FLAG_Z 0x080
#define CPU_FLAG_N 0x040
#define CPU_FLAG_H 0x020
#define CPU_FLAG_C 0x010

typedef struct sm83 {
	bus&			b;
	registers		reg{};
	uint8_t			curr_opcode{};
	instruction		curr_inst{};
	uint16_t		fetched_data{};
	bool			dest_is_mem{};
	uint16_t		mem_dest{};

	explicit sm83(bus& _b)
		: b(_b) {
		reg.pc = CPU_START_PC;
	}
} sm83;

void cpu_step(
	sm83& c);

uint16_t cpu_read_reg(
	const sm83& c,
	reg_type r);

void cpu_write_reg(
	sm83& c,
	reg_type r,
	uint16_t value);

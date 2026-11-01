#pragma once
#include <Common.h>
#include <Bus.h>

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

enum inst_type : uint8_t {
	inst_nop = 0,
	inst_stop,
	inst_ld,
	inst_add,
	inst_jp,
};

enum addr_mode : uint8_t {
	adm_none = 0,
	adm_r8_r8,
	adm_n16,
	adm_r16_r8,
	adm_r16_n16,
};

enum reg_type : uint8_t {
	reg_none = 0,
	reg_a,
	reg_f,
	reg_b,
	reg_c,
	reg_d,
	reg_e,
	reg_h,
	reg_l,
	reg_af,
	reg_bc,
	reg_de,
	reg_hl,
	reg_sp,
	reg_pc,
};

enum cond_type : uint8_t {
	cond_none = 0,
	cond_z,
	cond_nz,
	cond_c,
	cond_nc,
};

typedef struct instruction {
	inst_type		type{};
	addr_mode		mode{};
	reg_type		r1{};
	reg_type		r2{};
	cond_type		cond{};
	uint8_t			param{};
} instruction;

typedef struct registers {
	uint8_t		a{};
	uint8_t		f{};
	uint8_t		b{};
	uint8_t		c{};
	uint8_t		d{};
	uint8_t		e{};
	uint8_t		h{};
	uint8_t		l{};
	uint16_t	sp{};
	uint16_t	pc{};
} registers;

typedef struct sm83 {
	bus&			b;
	registers		reg{};
	uint8_t			curr_opcode{};
	instruction		curr_inst{};
	uint16_t		fetched_data{};
	uint16_t		mem_dest{};
	bool			dest_is_mem{};

	explicit sm83(bus& _b)
		: b(_b) {
		reg.pc = CPU_START_PC;
	}
} sm83;

void cpu_step(
	sm83& c);

uint8_t cpu_read_r8(
	const sm83& c,
	reg_type r);

uint16_t cpu_read_r16(
	const sm83& c,
	reg_type r);

void cpu_write_r8(
	sm83& c,
	reg_type r,
	uint8_t value);

void cpu_write_r16(
	sm83& c,
	reg_type r,
	uint16_t value);

#pragma once
#include <Common.h>

struct sm83;

enum inst_type : uint8_t {
	inst_undefined = 0,
	inst_nop,
	inst_ld,
	inst_dec,
	inst_add,
	inst_xor,
	inst_jp,
	inst_jr,
	inst_stop,
};

enum addr_mode : uint8_t {
	adm_none = 0,
	adm_r,
	adm_r_r,
	adm_mr,
	adm_mr_r,
	adm_r_mr,
	adm_n8,
	adm_n16,
	adm_r_n8,
	adm_r_n16,
	adm_mr_n8,
	adm_a16_r,
	adm_r_a16,
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
	int8_t			mem_adj{};
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

inline bool cpu_is_r16(reg_type t) {
	return t >= reg_af;
}

const instruction& inst_from_opcode(
	uint8_t opcode);

std::string cpu_inst_to_str(
	const sm83& c);

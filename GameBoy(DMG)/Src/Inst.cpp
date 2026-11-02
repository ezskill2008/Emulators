#include <Inst.h>
#include <Cpu.h>

constexpr static instruction instrs[256]{
	{ inst_nop, adm_none, reg_none },						//0x00 NOOP			- 4
	{ inst_ld, adm_r_n16, reg_bc },							//0x01 LD BC, n16	- 12
	{ inst_ld, adm_mr_r, reg_bc, reg_a },					//0x02 LD [BC], A	- 8
	{},//0x03
	{},//0x04
	{ inst_dec, adm_r, reg_b, reg_none },					//0x05 DEC B		- 4
	{ inst_ld, adm_r_n8, reg_b, reg_none },					//0x06 LD B, n8		- 4
	{},//0x07
	{},//0x08
	{},//0x09
	{},//0x0a
	{},//0x0b
	{},//0x0c
	{},//0x0d
	{ inst_ld, adm_r_n8, reg_c, reg_none },					//0x0e LD C, n8		- 4
	{},//0x0f
	{},//0x10
	{},//0x11
	{},//0x12
	{ inst_ld, adm_r_n16, reg_sp, reg_none },				//0x13 LD SP, n16	- 12
	{},//0x14
	{},//0x15
	{},//0x16
	{},//0x17
	{},//0x18
	{},//0x19
	{},//0x1a
	{},//0x1b
	{},//0x1c
	{},//0x1d
	{},//0x1e
	{},//0x1f
	{ inst_jr, adm_n8, reg_none, reg_none, cond_nz },		//0x20 JR NZ, e8	- 12/8
	{ inst_ld, adm_r_n16, reg_hl, reg_none },				//0x21 LD HL, n16	- 12
	{},//0x22
	{},//0x23
	{},//0x24
	{},//0x25
	{},//0x26
	{},//0x27
	{},//0x28
	{},//0x29
	{},//0x2a
	{},//0x2b
	{},//0x2c
	{},//0x2d
	{},//0x2e
	{},//0x2f
	{},//0x30
	{},//0x31
	{ inst_ld, adm_mr_r, reg_hl, reg_a, cond_none, 0, -1 },	//0x32 LD [HL-], A	- 8
	{},//0x33
	{},//0x34
	{},//0x35
	{},//0x36
	{},//0x37
	{},//0x38
	{},//0x39
	{},//0x3a
	{},//0x3b
	{},//0x3c
	{},//0x3d
	{},//0x3e
	{},//0x3f
	{},//0x40
	{},//0x41
	{},//0x42
	{},//0x43
	{},//0x44
	{},//0x45
	{},//0x46
	{},//0x47
	{},//0x48
	{},//0x49
	{},//0x4a
	{},//0x4b
	{},//0x4c
	{},//0x4d
	{},//0x4e
	{},//0x4f
	{},//0x50
	{},//0x51
	{},//0x52
	{},//0x53
	{},//0x54
	{},//0x55
	{},//0x56
	{},//0x57
	{},//0x58
	{},//0x59
	{},//0x5a
	{},//0x5b
	{},//0x5c
	{},//0x5d
	{},//0x5e
	{},//0x5f
	{},//0x60
	{},//0x61
	{},//0x62
	{},//0x63
	{},//0x64
	{},//0x65
	{},//0x66
	{},//0x67
	{},//0x68
	{},//0x69
	{},//0x6a
	{},//0x6b
	{},//0x6c
	{},//0x6d
	{},//0x6e
	{},//0x6f
	{},//0x70
	{},//0x71
	{},//0x72
	{},//0x73
	{},//0x74
	{},//0x75
	{},//0x76
	{},//0x77
	{},//0x78
	{},//0x79
	{},//0x7a
	{},//0x7b
	{},//0x7c
	{},//0x7d
	{},//0x7e
	{},//0x7f
	{},//0x80
	{},//0x81
	{},//0x82
	{},//0x83
	{},//0x84
	{},//0x85
	{},//0x86
	{},//0x87
	{},//0x88
	{},//0x89
	{},//0x8a
	{},//0x8b
	{},//0x8c
	{},//0x8d
	{},//0x8e
	{},//0x8f
	{},//0x90
	{},//0x91
	{},//0x92
	{},//0x93
	{},//0x94
	{},//0x95
	{},//0x96
	{},//0x97
	{},//0x98
	{},//0x99
	{},//0x9a
	{},//0x9b
	{},//0x9c
	{},//0x9d
	{},//0x9e
	{},//0x9f
	{},//0xa0
	{},//0xa1
	{},//0xa2
	{},//0xa3
	{},//0xa4
	{},//0xa5
	{},//0xa6
	{},//0xa7
	{},//0xa8
	{},//0xa9
	{},//0xaa
	{},//0xab
	{},//0xac
	{},//0xad
	{},//0xae
	{ inst_xor, adm_r_r, reg_a, reg_a },				//0xaf XOR A, A		- 4
	{},//0xb0
	{},//0xb1
	{},//0xb2
	{},//0xb3
	{},//0xb4
	{},//0xb5
	{},//0xb6
	{},//0xb7
	{},//0xb8
	{},//0xb9
	{},//0xba
	{},//0xbb
	{},//0xbc
	{},//0xbd
	{},//0xbe
	{},//0xbf
	{},//0xc0
	{},//0xc1
	{},//0xc2
	{ inst_jp, adm_n16, reg_none, reg_none, cond_nz },	//0xc3 JP a16	- 16/12
	{},//0xc4
	{},//0xc5
	{},//0xc6
	{},//0xc7
	{},//0xc8
	{},//0xc9
	{},//0xca
	{},//0xcb
	{},//0xcc
	{},//0xcd
	{},//0xce
	{},//0xcf
	{},//0xd0
	{},//0xd1
	{},//0xd2
	{},//0xd3
	{},//0xd4
	{},//0xd5
	{},//0xd6
	{},//0xd7
	{},//0xd8
	{},//0xd9
	{},//0xda
	{},//0xdb
	{},//0xdc
	{},//0xdd
	{},//0xde
	{},//0xdf
	{},//0xe0
	{},//0xe1
	{},//0xe2
	{},//0xe3
	{},//0xe4
	{},//0xe5
	{},//0xe6
	{},//0xe7
	{},//0xe8
	{},//0xe9
	{},//0xea
	{},//0xeb
	{},//0xec
	{},//0xed
	{},//0xee
	{},//0xef
	{},//0xf0
	{},//0xf1
	{},//0xf2
	{},//0xf3
	{},//0xf4
	{},//0xf5
	{},//0xf6
	{},//0xf7
	{},//0xf8
	{},//0xf9
	{},//0xfa
	{},//0xfb
	{},//0xfc
	{},//0xfd
	{},//0xfe
	{}//0xff
};

const instruction&
inst_from_opcode(uint8_t opcode) {
	return instrs[opcode];
}

void
cpu_ss_hex(
	std::stringstream& ss,
	uint32_t hex,
	uint32_t width) {
	ss << '$'
		<< std::hex
		<< std::uppercase
		<< std::setw(width)
		<< std::setfill('0')
		<< hex;
}

void
cpu_ss_reg(std::stringstream& ss, reg_type r) {
	static constexpr const char* reg_str[]{
		"NONE", "A", "F", "B", "C", "D", "E", "H", "L",
		"AF", "BC", "DE", "HL", "SP", "PC",
	};
	ss << reg_str[r];
}

void
cpu_ss_mreg(std::stringstream& ss, reg_type r, int8_t adj) {
	ss << '[';
	cpu_ss_reg(ss, r);
	if (adj > 0) ss << '+';
	else if (adj < 0) ss << '-';
	ss << ']';
}

void
cpu_ss_val(std::stringstream& ss, uint16_t v, int digits) {
	cpu_ss_hex(ss, v, digits);
}

void
cpu_ss_addr(std::stringstream& ss, uint16_t v, int digits) {
	ss << "[$";
	cpu_ss_hex(ss, v, digits);
	ss << ']';
}

std::string
cpu_inst_to_str(
	const sm83& c) {
	static constexpr const char* op_str[]{
		"Undefined Instruction Op=",
		"NOP",
		"LD",
		"DEC",
		"ADD",
		"XOR",
		"JP",
		"JR",
		"STOP",
	};

	std::stringstream ss{};
	const instruction& i{ c.curr_inst };

	ss << op_str[i.type];

	if (i.type == inst_undefined) {
		cpu_ss_hex(ss, c.curr_opcode, 2);
		return ss.str();
	}

	if (i.mode != adm_none)
		ss << ' ';

	switch (i.mode) {
	case adm_r:
		cpu_ss_reg(ss, i.r1);
		break;
	case adm_r_r:
		cpu_ss_reg(ss, i.r1); ss << ", "; cpu_ss_reg(ss, i.r2);
		break;
	case adm_mr:
		cpu_ss_mreg(ss, i.r1, i.mem_adj);
		break;
	case adm_mr_r:
		cpu_ss_mreg(ss, i.r1, i.mem_adj); ss << ", "; cpu_ss_reg(ss, i.r2);
		break;
	case adm_r_mr:
		cpu_ss_reg(ss, i.r1); ss << ", "; cpu_ss_mreg(ss, i.r2, i.mem_adj);
		break;
	case adm_n8:
		cpu_ss_val(ss, c.fetched_data, 2);
		break;
	case adm_n16:
		cpu_ss_val(ss, c.fetched_data, 4);
		break;
	case adm_r_n8:
		cpu_ss_reg(ss, i.r1); ss << ", "; cpu_ss_val(ss, c.fetched_data, 2);
		break;
	case adm_r_n16:
		cpu_ss_reg(ss, i.r1); ss << ", "; cpu_ss_val(ss, c.fetched_data, 4);
		break;
	case adm_mr_n8:
		cpu_ss_mreg(ss, i.r1, i.mem_adj); ss << ", "; cpu_ss_val(ss, c.fetched_data, 2);
		break;
	case adm_a16_r:
		cpu_ss_addr(ss, c.fetched_data, 4); ss << ", "; cpu_ss_reg(ss, i.r2);
		break;
	case adm_r_a16:
		cpu_ss_reg(ss, i.r1); ss << ", "; cpu_ss_addr(ss, c.fetched_data, 4);
		break;
	default: break;
	}

	return ss.str();
}

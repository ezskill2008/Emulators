#include <Cpu.h>
#include <Timer.h>

bool
cpu_cond(sm83& c) {
	bool bz{ (c.reg.f & 0x80) != 0 };
	bool bc{ (c.reg.f & 0x10) != 0 };

	switch (c.curr_inst.cond) {
	case cond_c:	return  bc;
	case cond_nc:	return !bc;
	case cond_z:	return  bz;
	case cond_nz:	return !bz;
	}

	return false;
}

void
cpu_nop(sm83& c) {//FINAL
	UNREFERENCED_PARAMETER(c);
	timer_tcycles(4);
}

void
cpu_ld(sm83& c) {
	if (c.dest_is_mem) {
		uint16_t addr{ (uint16_t)(c.mem_dest + c.curr_inst.mem_adj) };
		bus_write(c.b, addr, (uint8_t)c.fetched_data);

		return;
	}

	cpu_write_reg(c, c.curr_inst.r1, c.fetched_data);
}

void
cpu_dec(sm83& c) {
	if (c.dest_is_mem) {
		uint8_t value{ bus_read(c.b, c.mem_dest) };
		--value;
		bus_write(c.b, c.mem_dest, value);

		BIT_SET(c.reg.f, 7, (value == 0));
		BIT_SET(c.reg.f, 6, 1);
		BIT_SET(c.reg.f, 5, ((value & 0x0F) == 0xF));
	}
	else if(cpu_is_r16(c.curr_inst.r1)) {
		uint16_t value{ c.fetched_data };
		--value;
		cpu_write_reg(c, c.curr_inst.r1, value);
	}
	else {
		uint8_t value{ (uint8_t)c.fetched_data };
		--value;
		cpu_write_reg(c, c.curr_inst.r1, value);

		BIT_SET(c.reg.f, 7, (value == 0));
		BIT_SET(c.reg.f, 6, 1);
		BIT_SET(c.reg.f, 5, ((value & 0x0F) == 0xF));
	}
}

void
cpu_xor(sm83& c) {
	uint16_t v{ cpu_read_reg(c, c.curr_inst.r1) };
	v ^= c.fetched_data;
	cpu_write_reg(c, reg_f, (v == 0) ? 0x80 : 0x00);
}

void
cpu_jp(sm83& c) {
	if (c.curr_inst.cond == cond_none || cpu_cond(c)) {
		cpu_write_reg(c, reg_pc, c.fetched_data);
	}
	else {
	}
}

void
cpu_jr(sm83& c) {
	if (c.curr_inst.cond == cond_none || cpu_cond(c)) {
		uint16_t pc{ cpu_read_reg(c, reg_pc) };
		pc += (int8_t)c.fetched_data;
		cpu_write_reg(c, reg_pc, pc);
	}
}

void
cpu_fetch(sm83& c) {
	c.dest_is_mem = false;
	c.mem_dest = 0;
	c.fetched_data = 0;

	switch (c.curr_inst.mode) {
	case adm_r: {
		c.fetched_data = cpu_read_reg(c, c.curr_inst.r1);
	} break;
	case adm_r_r: {
		c.fetched_data = cpu_read_reg(c, c.curr_inst.r2);
	} break;
	case adm_mr: {
		c.mem_dest = cpu_read_reg(c, c.curr_inst.r1);
		c.dest_is_mem = true;
		c.fetched_data = bus_read(c.b, c.mem_dest);
	} break;
	case adm_mr_r: {
		c.mem_dest = cpu_read_reg(c, c.curr_inst.r1);
		c.dest_is_mem = true;
		c.fetched_data = cpu_read_reg(c, c.curr_inst.r2);
	} break;
	case adm_r_mr: {
		c.fetched_data = bus_read(c.b, cpu_read_reg(c, c.curr_inst.r2));
	} break;
	case adm_r_n8:
	case adm_n8: {
		c.fetched_data = bus_read(c.b, c.reg.pc++);
	} break;
	case adm_n16:
	case adm_r_n16: {
		uint8_t lo{ bus_read(c.b, c.reg.pc++) };
		uint8_t hi{ bus_read(c.b, c.reg.pc++) };

		c.fetched_data = lo | (hi << 8);
	} break;
	default: return;
	}
}

void
cpu_step(
	sm83& c) {
	c.curr_opcode = bus_read(c.b, c.reg.pc++);
	c.curr_inst = inst_from_opcode(c.curr_opcode);

	cpu_fetch(c);

	switch (c.curr_inst.type) {
	case inst_nop:				break;
	case inst_ld:	cpu_ld(c);	break;
	case inst_dec:	cpu_dec(c);	break;
	case inst_xor:	cpu_xor(c);	break;
	case inst_jp:	cpu_jp(c);	break;
	case inst_jr:	cpu_jr(c);	break;
	default:
		break;
	}
}

uint16_t
cpu_read_reg(
	const sm83& c,
	reg_type r) {
	switch (r) {
	case reg_a: return (c.reg.a & 0xff);
	case reg_f: return (c.reg.f & 0xff);
	case reg_b: return (c.reg.b & 0xff);
	case reg_c: return (c.reg.c & 0xff);
	case reg_d: return (c.reg.d & 0xff);
	case reg_e: return (c.reg.e & 0xff);
	case reg_h: return (c.reg.h & 0xff);
	case reg_l: return (c.reg.l & 0xff);
	case reg_af: return ((uint16_t)c.reg.a) << 8 | (uint16_t)c.reg.f;
	case reg_bc: return ((uint16_t)c.reg.b) << 8 | (uint16_t)c.reg.c;
	case reg_de: return ((uint16_t)c.reg.d) << 8 | (uint16_t)c.reg.e;
	case reg_hl: return ((uint16_t)c.reg.h) << 8 | (uint16_t)c.reg.l;
	case reg_sp: return c.reg.sp;
	case reg_pc: return c.reg.pc;
	default: return 0;
	}
}

void
cpu_write_reg(
	sm83& c,
	reg_type r,
	uint16_t value) {
	switch (r) {
	case reg_a: { c.reg.a = value & 0xff; } break;
	case reg_f: { c.reg.f = value & 0xf0; } break;
	case reg_b: { c.reg.b = value & 0xff; } break;
	case reg_c: { c.reg.c = value & 0xff; } break;
	case reg_d: { c.reg.d = value & 0xff; } break;
	case reg_e: { c.reg.e = value & 0xff; } break;
	case reg_h: { c.reg.h = value & 0xff; } break;
	case reg_l: { c.reg.l = value & 0xff; } break;
	case reg_af: { c.reg.a = (value & 0xff00) >> 8; c.reg.f = value & 0xf0; } break;
	case reg_bc: { c.reg.b = (value & 0xff00) >> 8; c.reg.c = value & 0xff; } break;
	case reg_de: { c.reg.d = (value & 0xff00) >> 8; c.reg.e = value & 0xff; } break;
	case reg_hl: { c.reg.h = (value & 0xff00) >> 8; c.reg.l = value & 0xff; } break;
	case reg_sp: { c.reg.sp = value; } break;
	case reg_pc: { c.reg.pc = value; } break;
	default: return;
	}
}

void
cpu_ss_cond(
	std::stringstream& ss,
	cond_type c) {
	switch (c) {
	case cond_c:  ss << "C";  break;
	case cond_nc: ss << "NC"; break;
	case cond_z:  ss << "Z";  break;
	case cond_nz: ss << "NZ"; break;
	}
}

#include <Cartridge.h>

bool
ca_load_cartridge(
	cartridge& c,
	const char* path) {
	if (!(path && std::filesystem::exists(path)))
		return false;

	size_t file_size{ std::filesystem::file_size(path) };
	if (!file_size)
		return false;

	std::ifstream ifile{ path, std::ios::in | std::ios::binary };
	c.memory = std::make_unique<uint8_t[]>(file_size);
	ifile.read((char*)c.memory.get(), file_size);
	ifile.close();

	c.size = (uint32_t)file_size;

	return true;
}

bool
ca_parse_header(
	cartridge& c,
	cartridge_header& h,
	bool allow_cgb,
	bool dump) {
	if (c.size < 0x14F) {
		return false;
	}

	constexpr uint8_t logo_bytes[48]{
		0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
		0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
		0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
	};

	if (memcmp(&logo_bytes[0], &c.memory[0x104], 48) != 0) {
		if (dump) {
			printf("Error: invalid logo bytes!");
		}
		return false;
	}

	uint16_t title_end{ 0x143 };
	if (allow_cgb) {
		title_end = 0x142;
		
		h.cgb = c.memory[0x143] != 0;
	}

	h.title = { &c.memory[0x134], &c.memory[0x142] };

	switch (c.memory[0x147]) {
	case 0x01:	h.type = ca_mbc1; break;
	case 0x02:	h.type = ca_mbc1 | ca_ram; break;
	case 0x03:	h.type = ca_mbc1 | ca_ram | ca_battery; break;
	case 0x05:	h.type = ca_mbc2; break;
	case 0x06:	h.type = ca_mbc2 | ca_battery; break;
	case 0x08:	h.type = ca_rom | ca_ram; break;
	case 0x09:	h.type = ca_rom | ca_ram | ca_battery; break;
	case 0x0B:	h.type = ca_mmm01; break;
	case 0x0C:	h.type = ca_mmm01 | ca_ram; break;
	case 0x0D:	h.type = ca_mmm01 | ca_ram | ca_battery; break;
	case 0x0F:	h.type = ca_mbc3 | ca_timer | ca_battery; break;
	case 0x10:	h.type = ca_mbc3 | ca_timer | ca_ram | ca_battery; break;
	case 0x11:	h.type = ca_mbc3; break;
	case 0x12:	h.type = ca_mbc3 | ca_ram; break;
	case 0x13:	h.type = ca_mbc3 | ca_ram | ca_battery; break;
	case 0x19:	h.type = ca_mbc5; break;
	case 0x1A:	h.type = ca_mbc5 | ca_ram; break;
	case 0x1B:	h.type = ca_mbc5 | ca_ram | ca_battery; break;
	case 0x1C:	h.type = ca_mbc5 | ca_rumble; break;
	case 0x1D:	h.type = ca_mbc5 | ca_rumble | ca_ram; break;
	case 0x1E:	h.type = ca_mbc5 | ca_rumble | ca_ram | ca_battery; break;
	case 0x20:	h.type = ca_mbc6; break;
	case 0x22:	h.type = ca_mbc7 | ca_sensor | ca_rumble | ca_ram | ca_battery; break;
	case 0xFC:	h.type = ca_pocket_camera; break;
	case 0xFD:	h.type = ca_bandai_tama5; break;
	case 0xFE:	h.type = ca_huc3; break;
	case 0xFF:	h.type = ca_huc1 | ca_ram | ca_battery; break;
	default: break;
	}

	switch (c.memory[0x147]) {
	case 0x00: h.rom_size = 2 * 16 * 1024; break;
	case 0x01: h.rom_size = 4 * 16 * 1024; break;
	case 0x02: h.rom_size = 8 * 16 * 1024; break;
	case 0x03: h.rom_size = 16 * 16 * 1024; break;
	case 0x04: h.rom_size = 32 * 16 * 1024; break;
	case 0x05: h.rom_size = 64 * 16 * 1024; break;
	case 0x06: h.rom_size = 128 * 16 * 1024; break;
	case 0x07: h.rom_size = 256 * 16 * 1024; break;
	case 0x08: h.rom_size = 512 * 16 * 1024; break;
	case 0x52: h.rom_size = 72 * 16 * 1024; break;
	case 0x53: h.rom_size = 80 * 16 * 1024; break;
	case 0x54: h.rom_size = 95 * 16 * 1024; break;
	}

	if (h.type & ca_ram) {
		switch (c.memory[0x148]) {
		case 0x02: h.ram_size = 8 * 1024; break;
		case 0x03: h.ram_size = 32 * 1024; break;
		case 0x04: h.ram_size = 128 * 1024; break;
		case 0x05: h.ram_size = 64 * 1024; break;
		default: break;
		}
	}

	uint8_t checksum{ 0 };
	for (uint16_t address = 0x0134; address <= 0x014C; address++) {
		checksum = checksum - c.memory[address] - 1;
	}

	if (c.memory[0x14D] != checksum) {
		if (dump) {
			printf("Error: invalid checksum!");
		}
		return false;
	}

	if (dump) {
		printf("Loading cartridge success\n");
		printf("Game Title: %s\n", h.title.c_str());
		printf("Rom Size: %u\n", h.rom_size);
		printf("Ram Size: %u\n", h.ram_size);
		printf("Enable cgb: %s\n", h.cgb ? "True" : "False");
	}

	return true;
}

uint8_t
ca_read(
	cartridge& c,
	uint16_t addr) {
	return c.memory[addr];
}

void
ca_write(
	cartridge& c,
	uint16_t addr,
	uint8_t value) {
}


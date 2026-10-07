#include "cpu_8088.h"
#include <iomanip>

cpu_8088::cpu_8088() {
	RAM = new uint8_t[1024 * 1024];
	IP = 0x100;
	for (auto& s : segment_registers) {
		s = 0x0000;
	}
	registers[SP] = 0xFFFE;//SP
}

uint8_t cpu_8088::get_reg8(int index) {
	if (index < 4) {
		return registers[index] & 0xFF;
	}
	else {
		return (registers[index - 4] >> 8) & 0xFF;
	}
}

uint16_t& cpu_8088::get_reg16(int index) {
	return registers[index];
}

void cpu_8088::set_r8(int index, uint8_t value) {
	if (index < 4) {
		registers[index] = (registers[index] & 0xFF00) | value;
	}
	else {
		registers[index - 4] = (registers[index-4] & 0x00FF) | (value << 8);
	}
}

uint32_t cpu_8088::calc_addr(int mode, int rm) {
	uint16_t disp = 0;
	uint32_t res = 0;
	for (int i = 0; i < mode; i++) {
		disp <<= 8;
		disp += getRAM(IP + 1 + i,CS);
	}
	IP += mode;
	if (!(rm >> 2)) {
		if (!((rm >> 1) & 0b01)) {
			res = (segment_registers[changeSeg ? segment : DS] << 4) + get_reg16(BX) + get_reg16(SI + (rm) & 0b001) + disp;
		}
		else {
			res =  (segment_registers[changeSeg ? segment : SS] << 4) + get_reg16(BP) + get_reg16(SI + (rm) & 0b001) + disp;
		}
	}
	else {
		switch (rm)
		{
		case 0b100:
			res =  (segment_registers[changeSeg ? segment : DS] << 4) + get_reg16(SI) + disp;
			break;
		case 0b101:
			res =  (segment_registers[changeSeg ? segment : DS] << 4) + get_reg16(DI) + disp;
			break;
		case 0b110:
			res = (segment_registers[changeSeg ? segment : SS] << 4) + get_reg16(BP) + disp;
			break;
		case 0b111:
			res =  (segment_registers[changeSeg ? segment : DS] << 4) + get_reg16(BX) + disp;
			break;
		default:
			break;
		}
	}
	segment = DS;
	changeSeg = false;
	return res;
}

void cpu_8088::call_int(int num_int) {

}

cpu_8088::~cpu_8088() {
	delete[] RAM;
}

void cpu_8088::loadFileToRAM(FILE* file) {
	fseek(file,0,SEEK_END);
	int size = ftell(file);
	fseek(file, 0, SEEK_SET);
	fread(RAM+0x100, 1, size, file);
}

void cpu_8088::print_all_reg() {

	// 16-битные регистры
	std::cout << "AX: 0x" << std::hex << std::setw(4) << std::setfill('0') << get_reg16(AX) << "  ";
	std::cout << "CX: 0x" << std::hex << std::setw(4) << std::setfill('0') << get_reg16(CX) << "  ";
	std::cout << "DX: 0x" << std::hex << std::setw(4) << std::setfill('0') << get_reg16(DX) << "  ";
	std::cout << "BX: 0x" << std::hex << std::setw(4) << std::setfill('0') << get_reg16(BX) << '\n';

	std::cout << "SP: 0x" << std::hex << std::setw(4) << std::setfill('0') << get_reg16(SP) << "  ";
	std::cout << "BP: 0x" << std::hex << std::setw(4) << std::setfill('0') << get_reg16(BP) << "  ";
	std::cout << "SI: 0x" << std::hex << std::setw(4) << std::setfill('0') << get_reg16(SI) << "  ";
	std::cout << "DI: 0x" << std::hex << std::setw(4) << std::setfill('0') << get_reg16(DI) << '\n';

	std::cout << "----------------------------------------\n";

	// 8-битные регистры (вместе с 16-битными)
	std::cout << "AL: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)get_reg8(AL) << "  ";
	std::cout << "CL: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)get_reg8(CL) << "  ";
	std::cout << "DL: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)get_reg8(DL) << "  ";
	std::cout << "BL: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)get_reg8(BL) << "  ";
	std::cout << "AH: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)get_reg8(AH) << "  ";
	std::cout << "CH: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)get_reg8(CH) << "  ";
	std::cout << "DH: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)get_reg8(DH) << "  ";
	std::cout << "BH: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)get_reg8(BH) << '\n';

	std::cout << "----------------------------------------\n";

	// Сегментные регистры (используем segment_registers)
	std::cout << "ES: 0x" << std::hex << std::setw(4) << std::setfill('0') << segment_registers[ES] << "  ";
	std::cout << "CS: 0x" << std::hex << std::setw(4) << std::setfill('0') << segment_registers[CS] << "  ";
	std::cout << "SS: 0x" << std::hex << std::setw(4) << std::setfill('0') << segment_registers[SS] << "  ";
	std::cout << "DS: 0x" << std::hex << std::setw(4) << std::setfill('0') << segment_registers[DS] << '\n';

	std::cout << "----------------------------------------\n";

	// IP (Instruction Pointer)
	std::cout << "IP: 0x" << std::hex << std::setw(4) << std::setfill('0') << IP << '\n';

	std::cout << "----------------------------------------\n";

	// Флаги (побитово)
	std::cout << "Flags: ";
	std::cout << (flags & FLAG_CF ? "CF " : "   ");
	std::cout << (flags & FLAG_PF ? "PF " : "   ");
	std::cout << (flags & FLAG_AF ? "AF " : "   ");
	std::cout << (flags & FLAG_ZF ? "ZF " : "   ");
	std::cout << (flags & FLAG_SF ? "SF " : "   ");
	std::cout << (flags & FLAG_TF ? "TF " : "   ");
	std::cout << (flags & FLAG_IF ? "IF " : "   ");
	std::cout << (flags & FLAG_DF ? "DF " : "   ");
	std::cout << (flags & FLAG_OF ? "OF" : "   ");
	std::cout << '\n';

	std::cout << "========================================\n";

	// Возвращаем формат в десятичный (чтобы не испортить другие выводы)
	std::cout << std::dec;
}

void cpu_8088::emulate() {
	while (true) {
		uint8_t opcode = getRAM(IP, CS);
		uint8_t modeRM = getRAM(IP+1, CS);

		uint8_t mode = modeRM >> 6;
		uint8_t reg = (modeRM >> 3) & 7;
		uint8_t rm = modeRM & 7;
		int seg = 0;
		std::cout << "0x" << std::hex << int(opcode) << '\n';
		print_all_reg();
		switch (opcode) {
			//ADD
		case 0x00:
		case 0x01: {
			if (mode == 0b11) {
				if (opcode % 2 == 0) {
					uint8_t res = get_reg8(rm) + get_reg8(reg);
					update_flags_arifm(get_reg8(rm), get_reg8(reg), res, true);
					set_r8(rm, res);
				}
				else {
					uint16_t res = get_reg16(rm) + get_reg16(reg);
					update_flags_arifm(get_reg16(rm), get_reg16(reg), res, true);
					get_reg16(rm) += get_reg16(reg);
				}
				IP += 2;
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment]<<4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					if (opcode % 2 == 0) {
						uint8_t res = RAM[addr] + get_reg8(reg);
						update_flags_arifm(RAM[addr], get_reg8(reg), res, true);
						RAM[addr] = res;
					}
					else {
						uint16_t res = ((RAM[addr + 1] << 8) | RAM[addr]) + get_reg16(reg);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), get_reg16(reg), res, true);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
					}
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					if (opcode % 2 == 0) {
						uint8_t res = RAM[addr] + get_reg8(reg);
						update_flags_arifm(RAM[addr], get_reg8(reg), res, true);
						RAM[addr] = res;
					}
					else {
						uint16_t res = ((RAM[addr + 1] << 8) | RAM[addr]) + get_reg16(reg);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), get_reg16(reg), res, true);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
					}
					IP += 2;
				}
			}
			break;
		}
		case 0x02:
		case 0x03: {
			if (mode == 0b11) {
				if (opcode % 2 == 0) {
					uint8_t res = get_reg8(rm) + get_reg8(reg);
					update_flags_arifm(get_reg8(reg), get_reg8(rm), res, true);
					set_r8(reg, res);
				}
				else {
					uint16_t res = get_reg16(rm) + get_reg16(reg);
					update_flags_arifm(get_reg16(reg), get_reg16(rm), res, true);
					get_reg16(reg) += get_reg16(rm);
				}
				IP += 2;
				break;
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					if (opcode % 2 == 0) {
						uint8_t res = RAM[addr] + get_reg8(reg);
						update_flags_arifm(get_reg8(reg), RAM[addr], res, true);
						set_r8(reg, res);
					}
					else {
						uint16_t res = get_reg16(reg) + ((RAM[addr + 1] << 8) | RAM[addr]);
						update_flags_arifm(get_reg16(reg), uint16_t((RAM[addr + 1] << 8) | RAM[addr]), res, true);
						get_reg16(reg) += (RAM[addr + 1] << 8) | RAM[addr];
					}
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(reg) + RAM[addr];
						update_flags_arifm(get_reg8(reg), RAM[addr], res, true);
						set_r8(reg, res);
					}
					else {
						uint16_t res = get_reg16(reg) + ((RAM[addr + 1] << 8) | RAM[addr]);
						update_flags_arifm(get_reg16(reg), uint16_t((RAM[addr + 1] << 8) | RAM[addr]), res, true);
						get_reg16(reg) += (RAM[addr + 1] << 8) | RAM[addr];
					}
					IP += 2;
				}
			}
			break;
		}
		case 0x04:
		case 0x05: {
			if (opcode % 2 == 0) {
				uint8_t res = get_reg8(AL) + getRAM(IP + 1,CS);
				update_flags_arifm(get_reg8(AL), getRAM(IP + 1, CS), res, true);
				set_r8(AL, res);
				IP += 2;
			}
			else {
				uint16_t res = get_reg16(AX) + ((getRAM(IP + 2,CS) << 8) | getRAM(IP + 1,CS));
				update_flags_arifm(get_reg16(AX), uint16_t(((getRAM(IP + 2, CS) << 8) | getRAM(IP + 1, CS))), res, true);
				get_reg16(AX) = res;
				IP += 3;
			}
			break;
		}
		case 0x06:
			get_reg16(SP) -= 2;
			getRAM(SS, get_reg16(SP)) = segment_registers[ES];
			IP += 1;
			break;
		case 0x07:
			segment_registers[ES] = getRAM(SS, get_reg16(SP));
			get_reg16(SP) += 2;
			IP += 1;
			break;
		case 0x0E:
			get_reg16(SP) -= 2;
			getRAM(SS, get_reg16(SP)) = segment_registers[CS];
			IP += 1;
			break;
		case 0x16:
			get_reg16(SP) -= 2;
			getRAM(SS, get_reg16(SP)) = segment_registers[SS];
			IP += 1;
			break;
		case 0x17:
			segment_registers[SS] = getRAM(SS, get_reg16(SP));
			get_reg16(SP) += 2;
			IP += 1;
			break;
		case 0x1E:
			get_reg16(SP) -= 2;
			getRAM(SS, get_reg16(SP)) = segment_registers[DS];
			IP += 1;
			break;
		case 0x1F:
			segment_registers[DS] = getRAM(SS, get_reg16(SP));
			get_reg16(SP) += 2;
			IP += 1;
			break;
			//OR
		case 0x08:
		case 0x09:{
			if (mode == 0b11) {
				if (opcode % 2 == 0) {
					set_r8(rm, get_reg8(rm) | get_reg8(reg));
					update_flags_logic(get_reg8(rm));
					IP += 2;
				}
				else {
					get_reg16(rm) |= get_reg16(reg);
					update_flags_logic(get_reg16(rm));
					IP += 2;
				}
			}
			else {
				if (rm == 0b110) {
					uint16_t addr = (getRAM(IP+2, segment) << 8) | getRAM(IP+3, segment);
					if (opcode % 2 == 0) {
						uint8_t res = RAM[addr] | get_reg8(reg);
						RAM[addr] = res;
						update_flags_logic(RAM[addr]);
					}
					else {
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] | get_reg16(reg);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						update_flags_logic(res);
					}
					IP += 4;
				}
				else {
					uint16_t addr = calc_addr(mode, rm);
					if (opcode % 2 == 0) {
						RAM[addr] |= get_reg8(reg);
						IP += 2;
						update_flags_logic(RAM[addr]);
					}
					else {
						RAM[addr] |= get_reg16(reg) & 0xFF;
						RAM[addr + 1] |= (get_reg16(reg) >> 8) & 0xFF;
						update_flags_logic(uint16_t((RAM[addr+1] << 8) | RAM[addr]));
						IP += 2;
					}
				}
			}
			break;
		}
		case 0x0A:
		case 0x0B:{
			if (mode == 0b11) {
				if (opcode % 2 == 0) {
					set_r8(reg, get_reg8(reg) | get_reg8(rm));
					update_flags_logic(get_reg8(reg));
					IP += 2;
				}
				else {
					get_reg16(reg) |= get_reg16(rm);
					update_flags_logic(get_reg16(reg));
					IP += 2;
				}
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(reg) | RAM[addr];
						set_r8(reg, res);
						update_flags_logic(RAM[addr]);
					}
					else {
						get_reg16(reg) |= (RAM[addr + 1] << 8) | RAM[addr];
						update_flags_logic(get_reg16(reg));
					}
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(reg) | RAM[addr];
						set_r8(reg, res);
						update_flags_logic(RAM[addr]);
					}
					else {
						get_reg16(reg) |= (RAM[addr + 1] << 8) | RAM[addr];
						update_flags_logic(get_reg16(reg));
					}
				}
			}
			break;
		}
		case 0x0C:
		case 0x0D: {
			if (opcode % 2 == 0) {
				set_r8(AL, get_reg8(AL) | getRAM(IP+1,DS));
				update_flags_logic(get_reg8(AL));
				IP += 2;
			}
			else {
				get_reg16(AX) |= ((getRAM(IP+2,DS) << 8) | getRAM(IP+1,DS));
				update_flags_logic(get_reg16(AX));
				IP += 3;
			}
			break;
		} 
			//ADC(ADD+CF)
		case 0x10:
		case 0x11: {
			if (mode == 0b11) {
				if (opcode % 2 == 0) {
					uint8_t res = get_reg8(rm) + get_reg8(reg) + ((flags & FLAG_CF) ? 1 : 0);
					update_flags_arifm(get_reg8(rm), get_reg8(reg), res, true);
					set_r8(rm, res);
				}
				else {
					uint16_t res = get_reg16(rm) + get_reg16(reg) + ((flags & FLAG_CF) ? 1 : 0);
					update_flags_arifm(get_reg16(rm), get_reg16(reg), res, true);
					get_reg16(rm) += get_reg16(reg);
				}
				IP += 2;
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					if (opcode % 2 == 0) {
						uint8_t res = RAM[addr] + get_reg8(reg) + ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(RAM[addr], get_reg8(reg), res, true);
						RAM[addr] = res;
					}
					else {
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] + get_reg16(reg) + ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), get_reg16(reg), res, true);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
					}
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					if (opcode % 2 == 0) {
						uint8_t res = RAM[addr] + get_reg8(reg) + ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(RAM[addr], get_reg8(reg), res, true);
						RAM[addr] = res;
						IP += 2;
					}
					else {
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] + get_reg16(reg) + ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), get_reg16(reg), res, true);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						IP += 2;
					}
				}
			}
			break;
		}
		case 0x12:
		case 0x13: {
			if (mode == 0b11) {
				if (opcode % 2 == 0) {
					uint8_t res = get_reg8(rm) + get_reg8(reg) + ((flags & FLAG_CF) ? 1 : 0);
					update_flags_arifm(get_reg8(reg), get_reg8(rm), res, true);
					set_r8(reg, res);
				}
				else {
					uint16_t res = get_reg16(rm) + get_reg16(reg) + ((flags & FLAG_CF) ? 1 : 0);
					update_flags_arifm(get_reg16(reg), get_reg16(rm), res, true);
					get_reg16(reg) += get_reg16(rm);
				}
				IP += 2;
				break;
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					if (opcode % 2 == 0) {
						uint8_t res = RAM[addr] + get_reg8(rm) + ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(get_reg8(reg), RAM[addr], res, true);
						set_r8(reg, res);
					}
					else {
						uint16_t res = get_reg16(reg) + (RAM[addr + 1] << 8) | RAM[addr] + ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(get_reg16(reg), uint16_t((RAM[addr + 1] << 8) | RAM[addr]), res, true);
						get_reg16(reg) += (RAM[addr + 1] << 8) | RAM[addr];
					}
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(reg) + RAM[addr] + ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(get_reg8(reg), RAM[addr], res, true);
						set_r8(reg, res);
						IP += 3;
					}
					else {
						uint16_t res = get_reg16(reg) + (RAM[addr + 1] << 8) | RAM[addr] + ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(get_reg16(reg), uint16_t((RAM[addr + 1] << 8) | RAM[addr]), res, true);
						get_reg16(reg) += (RAM[addr + 1] << 8) | RAM[addr];
						IP += 4;
					}
				}
			}
			break;
		}
		case 0x14:
		case 0x15: {
			if (opcode % 2 == 0) {
				uint8_t res = get_reg8(AL) + getRAM(IP + 1, CS) + ((flags & FLAG_CF) ? 1 : 0);
				update_flags_arifm(get_reg8(AL), getRAM(IP + 1, CS), res, true);
				set_r8(AL, res);
				IP += 2;
			}
			else {
				uint16_t res = get_reg16(AX) + ((getRAM(IP + 2, CS) << 8) | getRAM(IP + 1, CS)) + ((flags & FLAG_CF) ? 1 : 0);
				update_flags_arifm(get_reg16(AX), uint16_t(((getRAM(IP + 2, CS) << 8) | getRAM(IP + 1, CS))), res, true);
				get_reg16(AX) = res;
				IP += 3;
			}
			break;
		}
			 //SBB(SUB-CF)
		case 0x18:
		case 0x19: {
			if (mode == 0b11) {
				if (opcode % 2 == 0) {
					uint8_t res = get_reg8(rm) - get_reg8(reg) - ((flags & FLAG_CF) ? 1 : 0);
					update_flags_arifm(get_reg8(rm), get_reg8(reg), res, false);
					set_r8(rm, res);
				}
				else {
					uint16_t res = get_reg16(rm) - get_reg16(reg) - ((flags & FLAG_CF) ? 1 : 0);
					update_flags_arifm(get_reg16(rm), get_reg16(reg), res, false);
					get_reg16(rm) = res;
				}
				IP += 2;
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					if (opcode % 2 == 0) {
						uint8_t res = RAM[addr] - get_reg8(reg) - ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(RAM[addr], get_reg8(reg), res, false);
						RAM[addr] = res;
					}
					else {
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] - get_reg16(reg) - ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), get_reg16(reg), res, false);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
					}
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					if (opcode % 2 == 0) {
						uint8_t res = RAM[addr] - get_reg8(reg) - ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(RAM[addr], get_reg8(reg), res, false);
						RAM[addr] = res;
					}
					else {
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] - get_reg16(reg) - ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), get_reg16(reg), res, false);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
					}
					IP += 2;
				}
			}
			break;
		}
		case 0x1A:
		case 0x1B: {
			if (mode == 0b11) {
				if (opcode % 2 == 0) {
					uint8_t res = get_reg8(reg) - get_reg8(rm) - ((flags & FLAG_CF) ? 1 : 0);
					update_flags_arifm(get_reg8(reg), get_reg8(rm), res, false);
					set_r8(reg, res);
				}
				else {
					uint16_t res = get_reg16(reg) - get_reg16(rm) - ((flags & FLAG_CF) ? 1 : 0);
					update_flags_arifm(get_reg16(reg), get_reg16(rm), res, false);
					get_reg16(reg) = res;
				}
				IP += 2;
				break;
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(reg) - RAM[addr] - ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(get_reg8(reg), RAM[addr], res, false);
						set_r8(reg, res);
					}
					else {
						uint16_t res = get_reg16(reg) - (RAM[addr + 1] << 8) | RAM[addr] - ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(get_reg16(reg), uint16_t((RAM[addr + 1] << 8) | RAM[addr]), res, false);
						get_reg16(reg) = res;
					}
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(reg) - RAM[addr] - ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(get_reg8(reg), RAM[addr], res, false);
						set_r8(reg, res);
					}
					else {
						uint16_t res = get_reg16(reg) - ((RAM[addr + 1] << 8) | RAM[addr]) - ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(get_reg16(reg), uint16_t((RAM[addr + 1] << 8) | RAM[addr]), res, false);
						get_reg16(reg) = res;
					}
					IP += 2;
				}
			}
			break;
		}
		case 0x1C:
		case 0x1D: {
			if (opcode % 2 == 0) {
				uint8_t res = get_reg8(AL) - getRAM(IP + 1, CS) - ((flags & FLAG_CF) ? 1 : 0);
				update_flags_arifm(get_reg8(AL), getRAM(IP + 1, CS), res, false);
				set_r8(AL, res);
				IP += 2;
			}
			else {
				uint16_t res = get_reg16(AX) - ((getRAM(IP + 2, CS) << 8) | getRAM(IP + 1, CS)) - ((flags & FLAG_CF) ? 1 : 0);
				update_flags_arifm(get_reg16(AX), uint16_t(((getRAM(IP + 2, CS) << 8) | getRAM(IP + 1, CS))), res, false);
				get_reg16(AX) = res;
				IP += 3;
			}
			break;
		}
			 //AND
		case 0x20:
		case 0x21: {
			if (mode == 0b11) {
				if (opcode % 2 == 0) {
					set_r8(rm, get_reg8(rm) & get_reg8(reg));
					update_flags_logic(get_reg8(rm));
					IP += 2;
				}
				else {
					get_reg16(rm) &= get_reg16(reg);
					update_flags_logic(get_reg16(rm));
					IP += 2;
				}
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					if (opcode % 2 == 0) {
						uint8_t res = RAM[addr] & get_reg8(reg);
						RAM[addr] = res;
						update_flags_logic(RAM[addr]);
					}
					else {
						uint16_t res = ((RAM[addr + 1] << 8) | RAM[addr]) & get_reg16(reg);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						update_flags_logic(res);
					}
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					if (opcode % 2 == 0) {
						RAM[addr] &= get_reg8(reg);
						IP += 2;
						update_flags_logic(RAM[addr]);
					}
					else {
						RAM[addr] &= get_reg16(reg) & 0xFF;
						RAM[addr + 1] &= (get_reg16(reg) >> 8) & 0xFF;
						update_flags_logic(uint16_t((RAM[addr + 1] << 8) | RAM[addr]));
						IP += 2;
					}
				}
			}
			break;
		}
		case 0x22:
		case 0x23: {
			if (mode == 0b11) {
				if (opcode % 2 == 0) {
					set_r8(reg, get_reg8(reg) & get_reg8(rm));
					update_flags_logic(get_reg8(reg));
					IP += 2;
				}
				else {
					get_reg16(reg) &= get_reg16(rm);
					update_flags_logic(get_reg16(reg));
					IP += 2;
				}
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(reg) & RAM[addr];
						set_r8(reg, res);
						update_flags_logic(RAM[addr]);
					}
					else {
						get_reg16(reg) &= ((RAM[addr + 1] << 8) | RAM[addr]);
						update_flags_logic(get_reg16(reg));
					}
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(reg) & RAM[addr];
						set_r8(reg, res);
						update_flags_logic(RAM[addr]);
					}
					else {
						get_reg16(reg) &= (RAM[addr + 1] << 8) | RAM[addr];
						update_flags_logic(get_reg16(reg));
					}
				}
			}
			break;
		}
		case 0x24:
		case 0x25: {
			if (opcode % 2 == 0) {
				set_r8(AL, get_reg8(AL) & getRAM(IP + 1, DS));
				update_flags_logic(get_reg8(AL));
				IP += 2;
			}
			else {
				get_reg16(AX) &= ((getRAM(IP + 2, DS) << 8) | getRAM(IP + 1, DS));
				update_flags_logic(get_reg16(AX));
				IP += 3;
			}
			break;
		}
		case 0x26:
			segment = ES;
			changeSeg = true;
			IP += 1;
			break;
		case 0x2E:
			segment = CS;
			changeSeg = true;
			IP += 1;
			break;
		case 0x36:
			segment = SS;
			changeSeg = true;
			IP += 1;
			break;
		case 0x3E:
			segment = DS;
			changeSeg = true;
			IP += 1;
			break;
			//DAA и DAS
		case 0x27:
		case 0x2F: {
			uint8_t al = get_reg8(AL);
			uint8_t low_nibl = al & 0x0F;
			uint8_t high_nibl = (al >> 4) & 0x0F;
			IP += 1;
			if (low_nibl > 9 || (flags & FLAG_AF)) {
				al += ((opcode & 0x0F) >> 3) ? -0x06 : 0x06;
				flags |= FLAG_AF;
			}
			else {
				flags &= ~FLAG_AF;
			}
			if (high_nibl > 9 || (flags & FLAG_CF)) {
				al += ((opcode & 0x0F) >> 3) ? -0x60 : 0x60;
			}
			else {
				flags &= ~FLAG_CF;
			}
			set_r8(AL, al);

			flags &= ~(FLAG_ZF | FLAG_SF | FLAG_PF);
			if (al == 0) flags |= FLAG_ZF;
			if (al & 0x80) flags |= FLAG_SF;

			int ones = 0;
			for (int i = 0; i < 8; i++) {
				if (al & (1 << i)) ones++;
			}
			if (ones % 2 == 0) flags |= FLAG_PF;

			break;
		}
			//SUB
		case 0x28:
		case 0x29: {
			if (mode == 0b11) {
				if (opcode % 2 == 0) {
					uint8_t res = get_reg8(rm) - get_reg8(reg);
					update_flags_arifm(get_reg8(rm), get_reg8(reg), res, false);
					set_r8(rm, res);
				}
				else {
					uint16_t res = get_reg16(rm) - get_reg16(reg);
					update_flags_arifm(get_reg16(rm), get_reg16(reg), res, false);
					get_reg16(rm) = res;
				}
				IP += 2;
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					if (opcode % 2 == 0) {
						uint8_t res = RAM[addr] - get_reg8(reg);
						update_flags_arifm(RAM[addr], get_reg8(reg), res, false);
						RAM[addr] = res;
					}
					else {
						uint16_t res = ((RAM[addr + 1] << 8) | RAM[addr]) - get_reg16(reg);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), get_reg16(reg), res, false);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
					}
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					if (opcode % 2 == 0) {
						uint8_t res = RAM[addr] - get_reg8(reg);
						update_flags_arifm(RAM[addr], get_reg8(reg), res, false);
						RAM[addr] = res;
					}
					else {
						uint16_t res = ((RAM[addr + 1] << 8) | RAM[addr]) - get_reg16(reg);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), get_reg16(reg), res, false);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
					}
					IP += 2;
				}
			}
			break;
		}
		case 0x2A:
		case 0x2B: {
			if (mode == 0b11) {
				if (opcode % 2 == 0) {
					uint8_t res = get_reg8(reg) - get_reg8(rm);
					update_flags_arifm(get_reg8(reg), get_reg8(rm), res, false);
					set_r8(reg, res);
				}
				else {
					uint16_t res = get_reg16(reg) - get_reg16(rm);
					update_flags_arifm(get_reg16(reg), get_reg16(rm), res, false);
					get_reg16(reg) = res;
				}
				IP += 2;
				break;
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					if (opcode % 2 == 0) {
						uint8_t res = RAM[addr] - get_reg8(reg);
						update_flags_arifm(get_reg8(reg), RAM[addr], res, false);
						set_r8(reg, res);
					}
					else {
						uint16_t res = get_reg16(reg) - ((RAM[addr + 1] << 8) | RAM[addr]);
						update_flags_arifm(get_reg16(reg), uint16_t((RAM[addr + 1] << 8) | RAM[addr]), res, false);
						get_reg16(reg) = res;
					}
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(reg) - RAM[addr];
						update_flags_arifm(get_reg8(reg), RAM[addr], res, false);
						set_r8(reg, res);
					}
					else {
						uint16_t res = get_reg16(reg) - ((RAM[addr + 1] << 8) | RAM[addr]);
						update_flags_arifm(get_reg16(reg), uint16_t((RAM[addr + 1] << 8) | RAM[addr]), res, false);
						get_reg16(reg) = res;
					}
					IP += 2;
				}
			}
			break;
		}
		case 0x2C:
		case 0x2D: {
			if (opcode % 2 == 0) {
				uint8_t res = get_reg8(AL) - getRAM(IP+1,CS);
				update_flags_arifm(get_reg8(AL), getRAM(IP + 1, CS), res, false);
				set_r8(AL, res);
				IP += 2;
			}
			else {
				uint16_t res = get_reg16(AX) - ((getRAM(IP + 1, CS) << 8) | getRAM(IP + 2, CS));
				update_flags_arifm(get_reg16(AX), uint16_t((getRAM(IP + 1, CS) << 8) | getRAM(IP + 2, CS)), res, false);
				get_reg16(AX) = res;
				IP += 3;
			}
			break;
		}
				 //XOR
		case 0x30:
		case 0x31: {
			if (mode == 0b11) {
				if (opcode % 2 == 0) {
					set_r8(rm, get_reg8(rm) ^ get_reg8(reg));
					update_flags_logic(get_reg8(rm));
					IP += 2;
				}
				else {
					get_reg16(rm) ^= get_reg16(reg);
					update_flags_logic(get_reg16(rm));
					IP += 2;
				}
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					if (opcode % 2 == 0) {
						uint8_t res = RAM[addr] ^ get_reg8(reg);
						RAM[addr] = res;
						update_flags_logic(RAM[addr]);
					}
					else {
						uint16_t res = ((RAM[addr + 1] << 8) | RAM[addr]) ^ get_reg16(reg);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						update_flags_logic(res);
					}
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					if (opcode % 2 == 0) {
						RAM[addr] ^= get_reg8(reg);
						IP += 2;
						update_flags_logic(RAM[addr]);
					}
					else {
						RAM[addr] ^= get_reg16(reg) & 0xFF;
						RAM[addr + 1] ^= (get_reg16(reg) >> 8) & 0xFF;
						update_flags_logic(uint16_t((RAM[addr + 1] << 8) | RAM[addr]));
						IP += 2;
					}
				}
			}
			break;
		}
		case 0x32:
		case 0x33: {
			if (mode == 0b11) {
				if (opcode % 2 == 0) {
					set_r8(reg, get_reg8(reg) ^ get_reg8(rm));
					update_flags_logic(get_reg8(reg));
					IP += 2;
				}
				else {
					get_reg16(reg) ^= get_reg16(rm);
					update_flags_logic(get_reg16(reg));
					IP += 2;
				}
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(reg) ^ RAM[addr];
						set_r8(reg, res);
						update_flags_logic(RAM[addr]);
					}
					else {
						get_reg16(reg) ^= (RAM[addr + 1] << 8) | RAM[addr];
						update_flags_logic(get_reg16(reg));
					}
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(reg) ^ RAM[addr];
						set_r8(reg, res);
						update_flags_logic(RAM[addr]);
					}
					else {
						get_reg16(reg) ^= (RAM[addr + 1] << 8) | RAM[addr];
						update_flags_logic(get_reg16(reg));
					}
				}
			}
			break;
		}
		case 0x34:
		case 0x35: {
			if (opcode % 2 == 0) {
				set_r8(AL, get_reg8(AL) ^ getRAM(IP + 1, DS));
				update_flags_logic(get_reg8(AL));
				IP += 2;
			}
			else {
				get_reg16(AX) ^= ((getRAM(IP + 2, DS) << 8) | getRAM(IP + 1, DS));
				update_flags_logic(get_reg16(AX));
				IP += 3;
			}
			break;
		}
				 //AAA и AAS
		case 0x37:
		case 0x3F: {
			IP += 1;
			uint8_t al = get_reg8(AL);
			uint8_t low_nibl = al & 0x0F;
			uint8_t high_nibl = (al>>4) & 0x0F;
			if (low_nibl > 9 || flags & FLAG_AF) {
				al += ((opcode & 0x0F)>>3) ? -0x06 : 0x06;
				set_r8(AH, get_reg8(AH) + ((opcode & 0x0F) >> 3) ? -1 : 1);
				flags |= (FLAG_AF | FLAG_CF);
			}
			else {
				flags &= ~(FLAG_AF | FLAG_CF);
			}
			al &= 0x0F;
			set_r8(AL, al);
			break;
		}
				 //CMP
		case 0x38:
		case 0x39: {
			if (mode == 0b11) {
				if (opcode % 2 == 0) {
					uint8_t res = get_reg8(rm) - get_reg8(reg);
					update_flags_arifm(get_reg8(rm), get_reg8(reg), res, false);
				}
				else {
					uint16_t res = get_reg16(rm) - get_reg16(reg);
					update_flags_arifm(get_reg16(rm), get_reg16(reg), res, false);
				}
				IP += 2;
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					if (opcode % 2 == 0) {
						uint8_t res = RAM[addr] - get_reg8(reg);
						update_flags_arifm(RAM[addr], get_reg8(reg), res, false);
					}
					else {
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] - get_reg16(reg);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), get_reg16(reg), res, false);
					}
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					if (opcode % 2 == 0) {
						uint8_t res = RAM[addr] - get_reg8(reg);
						update_flags_arifm(RAM[addr], get_reg8(reg), res, false);
						IP += 3;
					}
					else {
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] - get_reg16(reg);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), get_reg16(reg), res, false);
						IP += 4;
					}
				}
			}
			break;
		}
		case 0x3A:
		case 0x3B: {
			if (mode == 0b11) {
				if (opcode % 2 == 0) {
					uint8_t res = get_reg8(reg) - get_reg8(rm);
					update_flags_arifm(get_reg8(reg), get_reg8(rm), res, false);
				}
				else {
					uint16_t res = get_reg16(reg) - get_reg16(rm);
					update_flags_arifm(get_reg16(reg), get_reg16(rm), res, false);
				}
				IP += 2;
				break;
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(reg) - RAM[addr];
						update_flags_arifm(get_reg8(reg), RAM[addr], res, false);
					}
					else {
						uint16_t res = get_reg16(reg) - (RAM[addr + 1] << 8) | RAM[addr];
						update_flags_arifm(get_reg16(reg), uint16_t((RAM[addr + 1] << 8) | RAM[addr]), res, false);
					}
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(reg) - RAM[addr];
						update_flags_arifm(get_reg8(reg), RAM[addr], res, false);
						IP += 3;
					}
					else {
						uint16_t res = get_reg16(reg) - (RAM[addr + 1] << 8) | RAM[addr];
						update_flags_arifm(get_reg16(reg), uint16_t((RAM[addr + 1] << 8) | RAM[addr]), res, false);
						IP += 4;
					}
				}
			}
			break; 
		}
		case 0x3C:
		case 0x3D: {
			if (opcode % 2 == 0) {
				uint8_t res = get_reg8(AL) - getRAM(IP + 1, DS);
				update_flags_arifm(get_reg8(AL), getRAM(IP + 1, DS), res, false);
				IP += 2;
			}
			else {
				uint16_t res = get_reg16(AX) - ((getRAM(IP + 2, DS) << 8) | getRAM(IP + 1, DS));
				update_flags_arifm(get_reg16(AX), uint16_t((getRAM(IP + 2, DS) << 8) | getRAM(IP + 1, DS)), res, false);
				IP += 3;
			}
			break;
		}
				 //INC
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47: {
			IP += 1;
			get_reg16(opcode - 0x40)++;
			update_flags_inc(get_reg16(opcode - 0x40), true);
			break;
		}
				 //DEC
		case 0x48:
		case 0x49:
		case 0x4A:
		case 0x4B:
		case 0x4C:
		case 0x4D: 
		case 0x4E: 
		case 0x4F: {
			IP += 1;
			get_reg16(opcode - 0x48)--;
			update_flags_inc(get_reg16(opcode - 0x48), false);
			break;
		}
				// PUSH AX-SI
		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57: {
			IP += 1;
			get_reg16(SP) -= 2;
			RAM[(segment_registers[SS] << 4) | get_reg16(SP)] = get_reg16(opcode - 0x50) & 0xFF;
			RAM[((segment_registers[SS] << 4) | get_reg16(SP))+1] = (get_reg16(opcode - 0x50)>>8) & 0xFF;
			break;
		}
				 // POP AX-SI
		case 0x58:
		case 0x59:
		case 0x5A:
		case 0x5B:
		case 0x5C:
		case 0x5D:
		case 0x5E:
		case 0x5F: {
			IP += 1;
			get_reg16(opcode - 0x58) = (getRAM(get_reg16(SP) + 1, SS) << 8) | getRAM(get_reg16(SP), SS);
			get_reg16(SP) += 2;
			break;
		}
				 //Jump (0x60-0x6F alias 0x70-0x7F)
				 //JO
		case 0x60:
		case 0x70: {
			if (flags & FLAG_OF) {
				IP += RAM[(segment_registers[CS] << 4) | (IP + 1)];
				break;
			}
			IP += 2;
			break;
		}
				 //JNO
		case 0x61:
		case 0x71: {
			if (!(flags & FLAG_OF)) {
				IP += RAM[(segment_registers[CS] << 4) | (IP + 1)];
				break;
			}
			IP += 2;
			break;
		}
				 //JB
		case 0x62:
		case 0x72: {
			if (flags & FLAG_CF) {
				IP += RAM[(segment_registers[CS] << 4) | (IP + 1)];
				break;
			}
			IP += 2;
			break;
		}
				 //JAE
		case 0x63:
		case 0x73: {
			if (!(flags & FLAG_CF)) {
				IP += RAM[(segment_registers[CS] << 4) | (IP + 1)];
				break;
			}
			IP += 2;
			break;
		}
				 //JE
		case 0x64:
		case 0x74: {
			if (flags & FLAG_ZF) {
				IP += RAM[(segment_registers[CS] << 4) | (IP + 1)];
				break;
			}
			IP += 2;
			break;
		}
				 //JNE
		case 0x65:
		case 0x75: {
			if (!(flags & FLAG_ZF)) {
				IP += RAM[(segment_registers[CS] << 4) | (IP + 1)];
				break;
			}
			IP += 2;
			break;
		}
				 //JBE
		case 0x66:
		case 0x76: {
			if (flags & FLAG_ZF || flags & FLAG_CF) {
				IP += RAM[(segment_registers[CS] << 4) | (IP + 1)];
				break;
			}
			IP += 2;
			break;
		}
				 //JA
		case 0x67:
		case 0x77: {
			if (!(flags & FLAG_ZF) && !(flags & FLAG_CF)) {
				IP += RAM[(segment_registers[CS] << 4) | (IP + 1)];
				break;
			}
			IP += 2;
			break;
		}
				 //JS
		case 0x68:
		case 0x78: {
			if (flags & FLAG_SF) {
				IP += RAM[(segment_registers[CS] << 4) | (IP + 1)];
				break;
			}
			IP += 2;
			break;
		}
				 //JNS
		case 0x69:
		case 0x79: {
			if (!(flags & FLAG_SF)) {
				IP += RAM[(segment_registers[CS] << 4) | (IP + 1)];
				break;
			}
			IP += 2;
			break;
		}
				 //JP
		case 0x6A:
		case 0x7A: {
			if (flags & FLAG_PF) {
				IP += RAM[(segment_registers[CS] << 4) | (IP + 1)];
				break;
			}
			IP += 2;
			break;
		}
				 //JNP
		case 0x6B:
		case 0x7B: {
			if (!(flags & FLAG_PF)) {
				IP += RAM[(segment_registers[CS] << 4) | (IP + 1)];
				break;
			}
			IP += 2;
			break;
		}
				 //JL
		case 0x6C:
		case 0x7C: {
			if ((flags & FLAG_SF) != (flags & FLAG_OF)) {
				IP += RAM[(segment_registers[CS] << 4) | (IP + 1)];
				break;
			}
			IP += 2;
			break;
		}
				 //JGE
		case 0x6D:
		case 0x7D: {
			if ((flags & FLAG_SF) == (flags & FLAG_OF)) {
				IP += RAM[(segment_registers[CS] << 4) | (IP + 1)];
				break;
			}
			IP += 2;
			break;
		}
				 //JLE
		case 0x6E:
		case 0x7E: {
			if ((flags & FLAG_ZF) || ((flags & FLAG_SF) != (flags & FLAG_OF))) {
				IP += RAM[(segment_registers[CS] << 4) | (IP + 1)];
				break;
			}
			IP += 2;
			break;
		}
				 //JG
		case 0x6F:
		case 0x7F: {
			if (!(flags & FLAG_ZF) || ((flags & FLAG_SF) == (flags & FLAG_OF))) {
				IP += RAM[(segment_registers[CS] << 4) | (IP + 1)];
				break;
			}
			IP += 2;
			break;
		}
				 //Арифметические и логические операции с регистром\памятью и числом
		case 0x80:
		case 0x81:
		case 0x82: {
			switch (reg) {

				//ADD
			case 0b000: {
				if (mode == 0b11) {
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(rm) + getRAM(IP + 2, CS);
						update_flags_arifm(get_reg8(rm), getRAM(IP + 2, CS), res, true);
						IP += 3;
						set_r8(rm, res);
					}
					else {
						uint16_t b = ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						uint16_t res = get_reg16(rm) + b;
						update_flags_arifm(get_reg16(rm), b, res, true);
						IP += 4;
						get_reg16(rm) = res;
					}
					break;
				}
				else {
					if (rm == 0b110) {
						uint16_t addr = (getRAM(IP+2, CS) << 8) | getRAM(IP+3, CS);
						if (opcode % 2 == 0) {
							uint8_t res = RAM[addr] + getRAM(IP+4, CS);
							update_flags_arifm(RAM[addr], getRAM(IP + 4, CS), res, true);
							RAM[addr] = res;
							IP += 5;
						}
						else {
							uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] + uint16_t(((getRAM(IP + 5, CS)<<8) | getRAM(IP + 4, CS)));
							update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(((getRAM(IP + 5, CS) << 8) | getRAM(IP + 4, CS))), res, true);
							RAM[addr] = res & 0xFF;
							RAM[addr + 1] = (res >> 8) & 0xFF;
							IP += 6;
						}
					}
					else {
						uint16_t addr = calc_addr(mode, rm);
						if (opcode % 2 == 0) {
							uint8_t res = RAM[addr] + getRAM(IP + 2, CS);
							update_flags_arifm(RAM[addr], getRAM(IP + 2, CS), res, true);
							RAM[addr] = res;
							IP += 3;
						}
						else {
							uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] + uint16_t(((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS)));
							update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(((getRAM(IP + 3, CS)<<8) | getRAM(IP + 2, CS))), res, true);
							RAM[addr] = res & 0xFF;
							RAM[addr + 1] = (res >> 8) & 0xFF;
							IP += 4;
						}
					}
				}
				break;
			}

				//OR
			case 0b001: {
				if (mode == 0b11) {
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(rm) | getRAM(IP + 2, CS);
						set_r8(rm, res);
						update_flags_logic(get_reg8(rm));
						IP += 3;
					}
					else {
						uint16_t b = ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						uint16_t res = get_reg16(rm) | b;
						get_reg16(rm) = res;
						update_flags_logic(res);
						IP += 4;
					}
				}
				else {
					if (rm == 0b110) {
						uint16_t addr = (getRAM(IP+2, CS) << 8) | getRAM(IP+3, CS);
						if (opcode % 2 == 0) {
							uint8_t res = RAM[addr] | getRAM(IP+4, CS);
							RAM[addr] = res;
							update_flags_logic(res);
							IP += 5;
						}
						else {
							uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] | uint16_t(((getRAM(IP + 5, CS)<<8) | getRAM(IP + 4, CS)));
							RAM[addr] = res & 0xFF;
							RAM[addr + 1] = (res >> 8) & 0xFF;
							IP += 6;
							update_flags_logic(res);
						}
					}
					else {
						uint16_t addr = calc_addr(mode, rm);
						if (opcode % 2 == 0) {
							uint8_t res = RAM[addr] | getRAM(IP + 2, CS);
							RAM[addr] = res;
							IP += 3;
							update_flags_logic(RAM[addr]);
						}
						else {
							uint16_t res = uint16_t((RAM[addr + 1] << 8) | RAM[addr]) | (uint16_t(((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS))));
							RAM[addr] = res & 0xFF;
							RAM[addr + 1] = (res >> 8) & 0xFF;
							update_flags_logic(uint16_t((RAM[addr+1] << 8) | RAM[addr]));
							IP += 4;
						}
					}
				}
				break;
			}

				//ADC
			case 0b010: {
				if (mode == 0b11) {
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(rm) + getRAM(IP + 2, CS) + ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(get_reg8(rm), getRAM(IP + 2, CS), res, true);
						IP += 3;
						set_r8(rm, res);
					}
					else {
						uint16_t b = ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						uint16_t res = get_reg16(rm) + b + ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(get_reg16(rm), b, res, true);
						IP += 4;
						get_reg16(rm) = res;
					}
					break;
				}
				else {
					if (rm == 0b110) {
						uint16_t addr = (getRAM(IP + 2, CS) << 8) | getRAM(IP + 3, CS);
						if (opcode % 2 == 0) {
							uint8_t res = RAM[addr] + getRAM(IP + 4, CS) + ((flags & FLAG_CF) ? 1 : 0);
							update_flags_arifm(RAM[addr], getRAM(IP + 4, CS), res, true);
							RAM[addr] = res;
							IP += 5;
						}
						else {
							uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] + uint16_t(((getRAM(IP + 5, CS) << 8) | getRAM(IP + 4, CS))) + ((flags & FLAG_CF) ? 1 : 0);
							update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(((getRAM(IP + 5, CS) << 8) | getRAM(IP + 4, CS))), res, true);
							RAM[addr] = res & 0xFF;
							RAM[addr + 1] = (res >> 8) & 0xFF;
							IP += 6;
						}
					}
					else {
						uint16_t addr = calc_addr(mode, rm);
						if (opcode % 2 == 0) {
							uint8_t res = RAM[addr] + getRAM(IP + 2, CS) + ((flags & FLAG_CF) ? 1 : 0);
							update_flags_arifm(RAM[addr], getRAM(IP + 2, CS), res, true);
							RAM[addr] = res;
							IP += 3;
						}
						else {
							uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] + uint16_t(((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS))) + ((flags & FLAG_CF) ? 1 : 0);
							update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS))), res, true);
							RAM[addr] = res & 0xFF;
							RAM[addr + 1] = (res >> 8) & 0xFF;
							IP += 4;
						}
					}
				}
				break;
			}

				//SBB
			case 0b011: {
				if (mode == 0b11) {
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(rm) - getRAM(IP + 2, CS) - ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(get_reg8(rm), getRAM(IP + 2, CS), res, true);
						IP += 3;
						set_r8(rm, res);
					}
					else {
						uint16_t b = ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						uint16_t res = get_reg16(rm) - b - ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(get_reg16(rm), b, res, true);
						IP += 4;
						get_reg16(rm) = res;
					}
					break;
				}
				else {
					if (rm == 0b110) {
						uint16_t addr = (getRAM(IP + 2, CS) << 8) | getRAM(IP + 3, CS);
						if (opcode % 2 == 0) {
							uint8_t res = RAM[addr] - getRAM(IP + 4, CS) - ((flags & FLAG_CF) ? 1 : 0);
							update_flags_arifm(RAM[addr], getRAM(IP + 4, CS), res, true);
							RAM[addr] = res;
							IP += 5;
						}
						else {
							uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] - uint16_t(((getRAM(IP + 5, CS) << 8) | getRAM(IP + 4, CS))) - ((flags & FLAG_CF) ? 1 : 0);
							update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(((getRAM(IP + 5, CS) << 8) | getRAM(IP + 4, CS))), res, true);
							RAM[addr] = res & 0xFF;
							RAM[addr + 1] = (res >> 8) & 0xFF;
							IP += 6;
						}
					}
					else {
						uint16_t addr = calc_addr(mode, rm);
						if (opcode % 2 == 0) {
							uint8_t res = RAM[addr] - getRAM(IP + 2, CS) - ((flags & FLAG_CF) ? 1 : 0);
							update_flags_arifm(RAM[addr], getRAM(IP + 2, CS), res, true);
							RAM[addr] = res;
							IP += 3;
						}
						else {
							uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] - uint16_t(((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS))) - ((flags & FLAG_CF) ? 1 : 0);
							update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS))), res, true);
							RAM[addr] = res & 0xFF;
							RAM[addr + 1] = (res >> 8) & 0xFF;
							IP += 4;
						}
					}
				}
				break;
			}

				//AND
			case 0b100: {
				if (mode == 0b11) {
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(rm) & getRAM(IP + 2, CS);
						set_r8(rm, res);
						update_flags_logic(get_reg8(rm));
						IP += 3;
					}
					else {
						uint16_t b = ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						uint16_t res = get_reg16(rm) & b;
						get_reg16(rm) = res;
						update_flags_logic(res);
						IP += 4;
					}
				}
				else {
					if (rm == 0b110) {
						uint16_t addr = (getRAM(IP + 2, CS) << 8) | getRAM(IP + 3, CS);
						if (opcode % 2 == 0) {
							uint8_t res = RAM[addr] & getRAM(IP + 4, CS);
							RAM[addr] = res;
							update_flags_logic(res);
							IP += 5;
						}
						else {
							uint16_t res = ((RAM[addr + 1] << 8) | RAM[addr]) & uint16_t(((getRAM(IP + 5, CS) << 8) | getRAM(IP + 4, CS)));
							RAM[addr] = res & 0xFF;
							RAM[addr + 1] = (res >> 8) & 0xFF;
							IP += 6;
							update_flags_logic(res);
						}
					}
					else {
						uint16_t addr = calc_addr(mode, rm);
						if (opcode % 2 == 0) {
							uint8_t res = RAM[addr] & getRAM(IP + 2, CS);
							RAM[addr] = res;
							IP += 3;
							update_flags_logic(RAM[addr]);
						}
						else {
							uint16_t res = uint16_t((RAM[addr + 1] << 8) | RAM[addr]) & (uint16_t(((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS))));
							RAM[addr] = res & 0xFF;
							RAM[addr + 1] = (res >> 8) & 0xFF;
							update_flags_logic(uint16_t((RAM[addr+1] << 8) | RAM[addr]));
							IP += 4;
						}
					}
				}
				break;
			}

				//SUB
			case 0b101: {
				if (mode == 0b11) {
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(rm) - getRAM(IP + 2, CS);
						update_flags_arifm(get_reg8(rm), getRAM(IP + 2, CS), res, true);
						IP += 3;
						set_r8(rm, res);
					}
					else {
						uint16_t b = ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						uint16_t res = get_reg16(rm) - b;
						update_flags_arifm(get_reg16(rm), b, res, true);
						IP += 4;
						get_reg16(rm) = res;
					}
					break;
				}
				else {
					if (rm == 0b110) {
						uint16_t addr = (getRAM(IP + 2, CS) << 8) | getRAM(IP + 3, CS);
						if (opcode % 2 == 0) {
							uint8_t res = RAM[addr] - getRAM(IP + 4, CS);
							update_flags_arifm(RAM[addr], getRAM(IP + 4, CS), res, true);
							RAM[addr] = res;
							IP += 5;
						}
						else {
							uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] - uint16_t(((getRAM(IP + 5, CS) << 8) | getRAM(IP + 4, CS)));
							update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(((getRAM(IP + 5, CS) << 8) | getRAM(IP + 4, CS))), res, true);
							RAM[addr] = res & 0xFF;
							RAM[addr + 1] = (res >> 8) & 0xFF;
							IP += 6;
						}
					}
					else {
						uint16_t addr = calc_addr(mode, rm);
						if (opcode % 2 == 0) {
							uint8_t res = RAM[addr] - getRAM(IP + 2, CS);
							update_flags_arifm(RAM[addr], getRAM(IP + 2, CS), res, true);
							RAM[addr] = res;
							IP += 3;
						}
						else {
							uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] - uint16_t(((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS)));
							update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS))), res, true);
							RAM[addr] = res & 0xFF;
							RAM[addr + 1] = (res >> 8) & 0xFF;
							IP += 4;
						}
					}
				}
				break;
			}

				//XOR
			case 0b110: {
				if (mode == 0b11) {
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(rm) ^ getRAM(IP + 2, CS);
						set_r8(rm, res);
						update_flags_logic(get_reg8(rm));
						IP += 3;
					}
					else {
						uint16_t b = ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						uint16_t res = get_reg16(rm) ^ b;
						get_reg16(rm) = res;
						update_flags_logic(res);
						IP += 4;
					}
				}
				else {
					if (rm == 0b110) {
						uint16_t addr = (getRAM(IP + 2, CS) << 8) | getRAM(IP + 3, CS);
						if (opcode % 2 == 0) {
							uint8_t res = RAM[addr] ^ getRAM(IP + 4, CS);
							RAM[addr] = res;
							update_flags_logic(res);
							IP += 5;
						}
						else {
							uint16_t res = ((RAM[addr + 1] << 8) | RAM[addr]) ^ uint16_t(((getRAM(IP + 5, CS) << 8) | getRAM(IP + 4, CS)));
							RAM[addr] = res & 0xFF;
							RAM[addr + 1] = (res >> 8) & 0xFF;
							IP += 6;
							update_flags_logic(res);
						}
					}
					else {
						uint16_t addr = calc_addr(mode, rm);
						if (opcode % 2 == 0) {
							uint8_t res = RAM[addr] ^ getRAM(IP + 2, CS);
							RAM[addr] = res;
							IP += 3;
							update_flags_logic(RAM[addr]);
						}
						else {
							uint16_t res = uint16_t((RAM[addr + 1] << 8) | RAM[addr]) ^ (uint16_t(((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS))));
							RAM[addr] = res & 0xFF;
							RAM[addr + 1] = (res >> 8) & 0xFF;
							update_flags_logic(uint16_t((RAM[addr + 1] << 8) | RAM[addr]));
							IP += 4;
						}
					}
				}
				break;
			}

				//CMP
			case 0b111: {
				if (mode == 0b11) {
					if (opcode % 2 == 0) {
						uint8_t res = get_reg8(rm) - getRAM(IP + 2, CS);
						update_flags_arifm(get_reg8(rm), getRAM(IP + 2, CS), res, true);
						IP += 3;
					}
					else {
						uint16_t b = ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						uint16_t res = get_reg16(rm) - b;
						update_flags_arifm(get_reg16(rm), b, res, true);
						IP += 4;
					}
					break;
				}
				else {
					if (rm == 0b110) {
						uint16_t addr = (getRAM(IP + 2, CS) << 8) | getRAM(IP + 3, CS);
						if (opcode % 2 == 0) {
							uint8_t res = RAM[addr] - getRAM(IP + 4, CS);
							update_flags_arifm(RAM[addr], getRAM(IP + 4, CS), res, true);
							IP += 5;
						}
						else {
							uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] - uint16_t(((getRAM(IP + 5, CS) << 8) | getRAM(IP + 4, CS)));
							update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(((getRAM(IP + 5, CS) << 8) | getRAM(IP + 4, CS))), res, true);
							IP += 6;
						}
					}
					else {
						uint16_t addr = calc_addr(mode, rm);
						if (opcode % 2 == 0) {
							uint8_t res = RAM[addr] - getRAM(IP + 2, CS);
							update_flags_arifm(RAM[addr], getRAM(IP + 2, CS), res, true);
							IP += 3;
						}
						else {
							uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] - uint16_t(((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS)));
							update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS))), res, true);
							IP += 4;
						}
					}
				}
				break;
			}

			}
			break;
		}

				//сокращенная форма операций 16-битных регистров c 8-битными числами
		case 0x83: {
			switch (reg) {
				//ADD
			case 0b000: {
				if (mode == 0b11) {
					int8_t b = getRAM(IP + 2, CS);
					uint16_t res = get_reg16(rm) + uint16_t(b);
					update_flags_arifm(get_reg16(rm), uint16_t(b), res, true);
					IP += 3;
					get_reg16(rm) = res;
					break;
				}
				else {
					if (rm == 0b110) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						int8_t b = getRAM(IP + 4, CS);
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] + uint16_t(b);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(b), res, true);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						IP += 5;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						int8_t b = getRAM(IP + 2, CS);
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] + uint16_t(b);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(b), res, true);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						IP += 3;
		
					}
				}
				break;
			}

				//OR
			case 0b001: {
				if (mode == 0b11) {
					int8_t b =  getRAM(IP + 2, CS);
					uint16_t res = get_reg16(rm) | uint16_t(b);
					get_reg16(rm) = res;
					update_flags_logic(res);
					IP += 4;
				}
				else {
					if (rm == 0b110) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						int8_t b = getRAM(IP + 4, CS);
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] | uint16_t(b);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						IP += 6;
						update_flags_logic(res);
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						int8_t b = getRAM(IP + 2, CS);
						uint16_t res = uint16_t((RAM[addr + 1] << 8) | RAM[addr]) | uint16_t(b);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						update_flags_logic(uint16_t((RAM[addr + 1] << 8) | RAM[addr]));
						IP += 4;
					
					}
				}
				break;
			}

				//ADC
			case 0b010: {
				if (mode == 0b11) {
					int8_t b = getRAM(IP + 2, CS);
					uint16_t res = get_reg16(rm) + uint16_t(b) + ((flags & FLAG_CF) ? 1 : 0);
					update_flags_arifm(get_reg16(rm), uint16_t(b), res, true);
					IP += 3;
					get_reg16(rm) = res;
					break;
				}
				else {
					if (rm == 0b110) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						int8_t b = getRAM(IP + 4, CS);
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] + uint16_t(b) + ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(b), res, true);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						IP += 5;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						int8_t b = getRAM(IP + 2, CS);
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] + uint16_t(b) + ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(b), res, true);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						IP += 3;

					}
				}
				break;
			}

				//SBB
			case 0b011: {
				if (mode == 0b11) {
					int8_t b = getRAM(IP + 2, CS);
					uint16_t res = get_reg16(rm) - uint16_t(b) - ((flags & FLAG_CF) ? 1 : 0);
					update_flags_arifm(get_reg16(rm), uint16_t(b), res, true);
					IP += 3;
					get_reg16(rm) = res;
					break;
				}
				else {
					if (rm == 0b110) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						int8_t b = getRAM(IP + 4, CS);
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] - uint16_t(b) - ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(b), res, true);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						IP += 5;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						int8_t b = getRAM(IP + 2, CS);
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] - uint16_t(b) - ((flags & FLAG_CF) ? 1 : 0);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(b), res, true);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						IP += 3;

					}
				}
				break;
			}

				//AND
			case 0b100: {
				if (mode == 0b11) {
					int8_t b = getRAM(IP + 2, CS);
					uint16_t res = get_reg16(rm) & uint16_t(b);
					get_reg16(rm) = res;
					update_flags_logic(res);
					IP += 4;
				}
				else {
					if (rm == 0b110) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						int8_t b = getRAM(IP + 4, CS);
						uint16_t res = ((RAM[addr + 1] << 8) | RAM[addr]) & uint16_t(b);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						IP += 6;
						update_flags_logic(res);
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						int8_t b = getRAM(IP + 2, CS);
						uint16_t res = uint16_t((RAM[addr + 1] << 8) | RAM[addr]) & uint16_t(b);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						update_flags_logic(uint16_t((RAM[addr + 1] << 8) | RAM[addr]));
						IP += 4;

					}
				}
				break;
			}

				//SUB
			case 0b101: {
				if (mode == 0b11) {
					int8_t b = getRAM(IP + 2, CS);
					uint16_t res = get_reg16(rm) - uint16_t(b);
					update_flags_arifm(get_reg16(rm), uint16_t(b), res, true);
					IP += 3;
					get_reg16(rm) = res;
					break;
				}
				else {
					if (rm == 0b110) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						int8_t b = getRAM(IP + 4, CS);
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] - uint16_t(b);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(b), res, true);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						IP += 5;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						int8_t b = getRAM(IP + 2, CS);
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] - uint16_t(b);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(b), res, true);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						IP += 3;

					}
				}
				break;
			}

				//XOR
			case 0b110: {
				if (mode == 0b11) {
					int8_t b = getRAM(IP + 2, CS);
					uint16_t res = get_reg16(rm) ^ uint16_t(b);
					get_reg16(rm) = res;
					update_flags_logic(res);
					IP += 4;
				}
				else {
					if (rm == 0b110) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						int8_t b = getRAM(IP + 4, CS);
						uint16_t res = ((RAM[addr + 1] << 8) | RAM[addr]) ^ uint16_t(b);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						IP += 6;
						update_flags_logic(res);
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						int8_t b = getRAM(IP + 2, CS);
						uint16_t res = uint16_t((RAM[addr + 1] << 8) | RAM[addr]) ^ uint16_t(b);
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						update_flags_logic(uint16_t((RAM[addr + 1] << 8) | RAM[addr]));
						IP += 4;

					}
				}
				break;
			}

				//CMP
			case 0b111: {
				if (mode == 0b11) {
					int8_t b = getRAM(IP + 2, CS);
					uint16_t res = get_reg16(rm) - uint16_t(b);
					update_flags_arifm(get_reg16(rm), uint16_t(b), res, true);
					IP += 3;
					break;
				}
				else {
					if (rm == 0b110) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						int8_t b = getRAM(IP + 4, CS);
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] - uint16_t(b);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(b), res, true);
						IP += 5;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						int8_t b = getRAM(IP + 2, CS);
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr] - uint16_t(b);
						update_flags_arifm(uint16_t((RAM[addr + 1] << 8) | RAM[addr]), uint16_t(b), res, true);
						IP += 3;

					}
				}
				break;
			}
			}
		}

			//TEST r/m8 reg
		case 0x84: {
			if (mode == 0b11) {
					update_flags_logic(get_reg8(rm)& get_reg8(reg));
					IP += 2;
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					update_flags_logic(RAM[addr] & get_reg8(reg));
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					IP += 2;
					update_flags_logic(RAM[addr] & get_reg8(reg));
				}
			}
			break;
		}

			//TEST reg r/m8
		case 0x85: {
			if (mode == 0b11) {
					update_flags_logic(get_reg8(reg)& get_reg8(rm));
					IP += 2;
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					update_flags_logic(RAM[addr] & get_reg8(reg));
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					IP += 2;
					update_flags_logic(RAM[addr] & get_reg8(reg));
				}
			}
			break;
		}

			//XCHG 8-ми битный 
		case 0x86: {
			if (mode == 3) {
				uint8_t rm_reg = get_reg8(rm);
				uint8_t reg_reg = get_reg8(reg);
				set_r8(rm, reg_reg);
				set_r8(reg, rm_reg);
				IP += 2;
			}
			else {
				if (rm == 6) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					uint8_t rm_m = RAM[addr];
					uint8_t reg_reg = get_reg8(reg);
					RAM[addr] = reg_reg;
					set_r8(reg, rm_m);
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					uint8_t rm_m = RAM[addr];
					uint8_t reg_reg = get_reg8(reg);
					RAM[addr] = reg_reg;
					set_r8(reg, rm_m);
					IP += 2;
				}
			}
			break;
		}

			//XCHG 16-и битный 
		case 0x87: {
			if (mode == 3) {
				uint16_t rm_reg = get_reg16(rm);
				uint16_t reg_reg = get_reg16(reg);
				get_reg16(rm) = reg_reg;
				get_reg16(reg) = rm_reg;
				IP += 2;
			}
			else {
				if (rm == 6) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					uint16_t rm_m = (RAM[addr + 1] << 8) | RAM[addr];
					uint16_t reg_reg = get_reg16(reg);
					RAM[addr] = reg_reg & 0xFF;
					RAM[addr + 1] = (reg_reg >> 8) & 0xFF;
					get_reg16(reg) = rm_m;
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					uint16_t rm_m = (RAM[addr + 1] << 8) | RAM[addr];
					uint16_t reg_reg = get_reg16(reg);
					RAM[addr] = reg_reg & 0xFF;
					RAM[addr + 1] = (reg_reg >> 8) & 0xFF;
					get_reg16(reg) = rm_m;
					IP += 2;
				}
			}
			break;
		}

			// MOV
		case 0x88:
		case 0x89: {
			if (mode == 0b11) {
				if (opcode % 2 == 0) {
					set_r8(rm, get_reg8(reg));
				}
				else {
					uint16_t res = get_reg16(reg);
					get_reg16(rm) = res;
				}
				IP += 2;
			}
			else {
				if (rm == 0b110) {
					uint16_t addr = (RAM[IP + 2] << 8) | RAM[IP + 3];
					if (opcode % 2 == 0) {
						RAM[addr] = get_reg8(reg);
					}
					else {
						RAM[addr] = get_reg16(reg) & 0xFF;
						RAM[addr + 1] = (get_reg16(reg) >> 8) & 0xFF;
					}
					IP += 4;
				}
				else {
					uint16_t addr = calc_addr(mode, rm);
					if (opcode % 2 == 0) {
						RAM[addr] = get_reg8(reg);
					}
					else {
						RAM[addr] = get_reg16(reg) & 0xFF;
						RAM[addr + 1] = (get_reg16(reg) >> 8) & 0xFF;
					}
					IP += 2;
				}
			}
			break;
		}
		case 0x8A:
		case 0x8B: {
			if (mode == 0b11) {
				if (opcode % 2 == 0) {
					set_r8(reg, get_reg8(rm));
				}
				else {
					uint16_t res = get_reg16(rm);
					get_reg16(reg) = res;
				}
				IP += 2;
				break;
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					if (opcode % 2 == 0) {
						set_r8(reg, RAM[addr]);
					}
					else {
						get_reg16(reg) = (RAM[addr + 1] << 8) | RAM[addr];
					}
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					if (opcode % 2 == 0) {
						set_r8(reg,RAM[addr]);
						IP += 2;
					}
					else {
						get_reg16(reg) = (RAM[addr + 1] << 8) | RAM[addr];
						IP += 2;
					}
				}
			}
			break;
		}
		case 0x8C: {
			if (mode == 0b11) {
				get_reg16(rm) = segment_registers[reg];
				IP += 2;
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					RAM[addr] = segment_registers[reg] & 0xFF;
					RAM[addr + 1] = (segment_registers[reg] >> 8) & 0xFF;
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					RAM[addr] = segment_registers[reg] & 0xFF;
					RAM[addr + 1] = (segment_registers[reg] >> 8) & 0xFF;
					IP += 2;
				}
			}
			break;
		}

			// LEA
		case 0x8D: {
			if (mode == 0b11) {
				uint16_t res = get_reg16(rm);
				get_reg16(reg) = res;
				IP += 2;
				break;
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					get_reg16(reg) = addr;
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					get_reg16(reg) = addr;
					IP += 2;
				}
			}
			break;
		}
			//MOV Sreg r/m16
		case 0x8E: {
			if (mode == 0b11) {
				segment_registers[reg] = get_reg16(rm);
				IP += 2;
			}
			else {
				if (rm == 0b110) {
					uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
					segment_registers[reg] = (RAM[addr + 1] << 8) | RAM[addr] & 0xFF;
					IP += 4;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					segment_registers[reg] = (RAM[addr + 1] << 8) | RAM[addr] & 0xFF;
					IP += 2;
				}
			}
			break;
		}
			//POP r/m16
		case 0x8F: {
			if (reg == 0) {
				if (mode == 3) {
					get_reg16(rm) = (getRAM(get_reg16(SP) + 1, SS) << 8) | getRAM(get_reg16(SP), SS);
					get_reg16(SP) += 2;
					IP += 2;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						RAM[addr + 1] = getRAM(get_reg16(SP) + 1, SS);
						RAM[addr] = getRAM(get_reg16(SP), SS);
						get_reg16(SP) += 2;
						IP += 4;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						RAM[addr + 1] = getRAM(get_reg16(SP) + 1, SS);
						RAM[addr] = getRAM(get_reg16(SP), SS);
						get_reg16(SP) += 2;
						IP += 2;
					}
				}
			}
			else
				IP += 2;
			break;
		}

			//NOP
		case 0x90: {
			IP += 1;
			break;
		}

			//XCHG
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x94:
		case 0x95:
		case 0x96:
		case 0x97:{
			uint16_t reg_r = get_reg16(opcode - 0x90);
			uint16_t rAX = get_reg16(AX);
			get_reg16(opcode - 0x90) = rAX;
			get_reg16(AX) = reg_r;
			IP += 1;
			break;
		}

			//CBW
		case 0x98: {
			int8_t reg = get_reg8(AL);
			get_reg16(AX) = int16_t(reg);
			IP += 1;
			break;
		}		
			
			//CWD
		case 0x99: {
			int16_t ax = (int16_t)get_reg16(AX);
			int32_t res = (int32_t)ax;

			get_reg16(DX) = uint16_t(res >> 16);
			get_reg16(AX) = uint16_t(res & 0xFFFF);
			IP += 1;
			break;
		}

			//CALL
		case 0x9A: {
			get_reg16(SP) -= 2;
			getRAM(get_reg16(SP), SS) = segment_registers[CS] & 0xFF;
			getRAM(get_reg16(SP) + 1, SS) = (segment_registers[CS] >> 8) & 0xFF;
			get_reg16(SP) -= 2;
			getRAM(get_reg16(SP), SS) = IP & 0xFF;
			getRAM(get_reg16(SP)+1, SS) = (IP>>8) & 0xFF;
			uint16_t newIP = (getRAM(IP + 2, CS) << 8) | getRAM(IP+1, CS);
			uint16_t newCS = (getRAM(IP + 4, CS) << 8) | getRAM(IP+3, CS);
			IP = newIP;
			segment_registers[CS] = newCS;
			break;
		}
				 
			//WAIT 
		case 0x9B: {
			IP += 1;
			break; //(у нас нет FPU)
		}

			//PUSHF 
		case 0x9C: {
			get_reg16(SP) -= 2;
			getRAM(get_reg16(SP), SS) = flags & 0xFF;
			getRAM(get_reg16(SP)+1, SS) = (flags>>8) & 0xFF;
			IP += 1;
			break;
		}

			//POPF
		case 0x9D: {
			flags = (getRAM(get_reg16(SP) + 1, SS) << 8) | getRAM(get_reg16(SP), SS);
			get_reg16(SP) += 2;
			IP += 1;
			break;
		}

			//SAHF
		case 0x9E: {
			flags &= 0xFF00;
			flags |= get_reg8(AH);
			IP += 1;
			break;
		}

			//LAHF
		case 0x9F: {
			set_r8(AH, (flags & 0xFF));
			IP += 1;
			break;
		}

			//MOV AL/AH memoffset
		case 0xA0: 
		case 0xA1: {
			if (opcode % 2 == 0) {
				set_r8(AL, getRAM(getRAM(IP + 1, CS), segment));
				IP += 2;
				break;
			}
			else {
				get_reg16(AX) = getRAM((getRAM(IP + 2, CS) << 8) | getRAM(IP + 1, CS),segment);
				IP += 3;
				break;
			}
		}
			//MOV memoffset AL/AH 
		case 0xA2: 
		case 0xA3:{
			if (opcode % 2 == 0) {
				getRAM(getRAM(IP + 1, CS), segment) = get_reg8(AL);
				IP += 2;
				break;
			}
			else {
				getRAM((getRAM(IP + 2, CS) << 8) | getRAM(IP + 1, CS), segment) = get_reg16(AX);
				IP += 3;
				break;
			}
		}
			//MOVSB
		case 0xA4: {
			if (prefRep) {
				do {
					getRAM(get_reg16(DI), ES) = getRAM(get_reg16(SI), DS);
					if (flags & FLAG_DF) {
						get_reg16(DI)--;
						get_reg16(SI)--;
					}
					else {
						get_reg16(DI)++;
						get_reg16(SI)++;
					}
					if (get_reg16(CX) != 0) get_reg16(CX)--;
				} while (get_reg16(CX));
				IP += 1;
				prefRep = false;
				break;
			}
			getRAM(get_reg16(DI), ES) = getRAM(get_reg16(SI), DS);
			if (flags & FLAG_DF) {
				get_reg16(DI)--;
				get_reg16(SI)--;
			}
			else {
				get_reg16(DI)++;
				get_reg16(SI)++;
			}
			IP += 1;
			break;
		}
			//MOVSW
		case 0xA5: {
			if (prefRep) {
				do {
					getRAM(get_reg16(DI), ES) = getRAM(get_reg16(SI), DS);
					getRAM(get_reg16(DI) + 1, ES) = getRAM(get_reg16(SI) + 1, DS);
					if (flags & FLAG_DF) {
						get_reg16(DI) -= 2;
						get_reg16(SI) -= 2;
					}
					else {
						get_reg16(DI) += 2;
						get_reg16(SI) += 2;
					}
					if (get_reg16(CX) != 0) get_reg16(CX)--;
				} while (get_reg16(CX));
				IP += 1;
				prefRep = false;
				break;
			}
			getRAM(get_reg16(DI), ES) = getRAM(get_reg16(SI), DS);
			getRAM(get_reg16(DI) + 1, ES) = getRAM(get_reg16(SI) + 1, DS);
			if (flags & FLAG_DF) {
				get_reg16(DI) -= 2;
				get_reg16(SI) -= 2;
			}
			else {
				get_reg16(DI) += 2;
				get_reg16(SI) += 2;
			}
			IP += 1;
			break;
		}
			//CMPSB
		case 0xA6: {
			if (prefRep) {
				do {
					update_flags_arifm(getRAM(get_reg16(DI), ES), getRAM(get_reg16(SI), DS), uint8_t(getRAM(get_reg16(DI), ES) - getRAM(get_reg16(SI), DS)), false);
					if (flags & FLAG_DF) {
						get_reg16(DI)--;
						get_reg16(SI)--;
					}
					else {
						get_reg16(DI)++;
						get_reg16(SI)++;
					}
				} while (get_reg16(CX) && (flags & FLAG_ZF));
				IP += 1;
				prefRep = false;
				break;
			}
			update_flags_arifm(getRAM(get_reg16(DI), ES), getRAM(get_reg16(SI), DS), uint8_t(getRAM(get_reg16(DI), ES) - getRAM(get_reg16(SI), DS)), false);
			if (flags & FLAG_DF) {
				get_reg16(DI)--;
				get_reg16(SI)--;
			}
			else {
				get_reg16(DI)++;
				get_reg16(SI)++;
			}
			IP += 1;
			break;
		}

			//CMPSW
		case 0xA7: {
			if (prefRep) {
				do {
					uint16_t a = (getRAM(get_reg16(DI) + 1, ES) << 8) | getRAM(get_reg16(DI), ES);
					uint16_t b = (getRAM(get_reg16(SI) + 1, DS) << 8) | getRAM(get_reg16(SI), DS);
					uint16_t res = a - b;
					update_flags_arifm(a, b, res, false);
					if (flags & FLAG_DF) {
						get_reg16(DI) -= 2;
						get_reg16(SI) -= 2;
					}
					else {
						get_reg16(DI) += 2;
						get_reg16(SI) += 2;
					}
				} while (get_reg16(CX) && (flags & FLAG_ZF));
				IP += 1;
				break;
			}
			uint16_t a = (getRAM(get_reg16(DI) + 1, ES) << 8) | getRAM(get_reg16(DI), ES);
			uint16_t b = (getRAM(get_reg16(SI) + 1, DS) << 8) | getRAM(get_reg16(SI), DS);
			uint16_t res = a - b;
			update_flags_arifm(a, b, res, false);
			if (flags & FLAG_DF) {
				get_reg16(DI) -= 2;
				get_reg16(SI) -= 2;
			}
			else {
				get_reg16(DI) += 2;
				get_reg16(SI) += 2;
			}
			IP += 1;
			break;

		}

			//TEST
		case 0xA8: 
		case 0xA9:{
			if (opcode % 2 == 0) {
				update_flags_logic(get_reg8(AL) & getRAM(IP + 1, CS));
				IP += 2;
			}
			else {
				uint16_t mem = (getRAM(IP + 2, CS) << 8) | getRAM(IP + 1, CS);
				update_flags_logic(get_reg16(AX) & mem);
				IP += 3;
			}
			break;
		}

			//STOSB
		case 0xAA: {
			if (prefRep) {
				do {
					getRAM(get_reg16(DI), ES) = get_reg8(AL);
					if (flags & FLAG_DF) {
						get_reg16(DI)--;
					}
					else {
						get_reg16(DI)++;
					}
				} while (get_reg16(CX));
				IP += 1;
				prefRep = false;
				break;
			}
			getRAM(get_reg16(DI), ES) = get_reg8(AL);
			if (flags & FLAG_DF) {
				get_reg16(DI)--;
			}
			else {
				get_reg16(DI)++;
			}
			IP += 1;
			break;
		}

			//STOSW
		case 0xAB: {
			if (prefRep) {
				do {
					getRAM(get_reg16(DI), ES) = get_reg16(AX) & 0xFF;
					getRAM(get_reg16(DI) + 1, ES) = (get_reg16(AX) >> 8) & 0xFF;
					if (flags & FLAG_DF) {
						get_reg16(DI) -= 2;
					}
					else {
						get_reg16(DI) += 2;
					}
				} while (get_reg16(CX));
				IP += 1;
				prefRep = false;
				break;
			}
			getRAM(get_reg16(DI), ES) = get_reg16(AX) & 0xFF;
			getRAM(get_reg16(DI)+1, ES) = (get_reg16(AX)>>8) & 0xFF;
			if (flags & FLAG_DF) {
				get_reg16(DI)-=2;
			}
			else {
				get_reg16(DI)+=2;
			}
			IP += 1;
			break;
		}

			//LODSB
		case 0xAC: {
			if (prefRep) {
				do {
					set_r8(AL, get_reg16(SI) & 0xFF);
					if (flags & FLAG_DF) {
						get_reg16(SI)--;
					}
					else {
						get_reg16(SI)++;
					}
				} while (get_reg16(CX));
				IP += 1;
				prefRep = false;
				break;
			}
			set_r8(AL, get_reg16(SI)&0xFF);
			if (flags & FLAG_DF) {
				get_reg16(SI)--;
			}
			else {
				get_reg16(SI)++;
			}
			IP += 1;
			break;
		}

			//LODSW
		case 0xAD: {
			if (prefRep) {
				do {
					uint16_t si = get_reg16(SI);
					get_reg16(AX) = si;
					if (flags & FLAG_DF) {
						get_reg16(SI)--;
					}
					else {
						get_reg16(SI)++;
					}
				} while (get_reg16(CX));
				IP += 1;
				prefRep = false;
				break;
			}
			uint16_t si = get_reg16(SI);
			get_reg16(AX) = si;
			if (flags & FLAG_DF) {
				get_reg16(SI)--;
			}
			else {
				get_reg16(SI)++;
			}
			IP += 1;
			break;
		}

			//SCASB
		case 0xAE: {
			if (prefRep) {
				do {
					update_flags_arifm(get_reg8(AL), getRAM(get_reg16(DI), ES), uint8_t(get_reg8(AL) - getRAM(get_reg16(DI), ES)), false);
					if (flags & FLAG_DF) {
						get_reg16(DI)--;
					}
					else {
						get_reg16(DI)++;
					}
				} while (get_reg16(CX) && (flags & FLAG_ZF));
				IP += 1;
				prefRep = false;
				break;
			}
			update_flags_arifm(get_reg8(AL), getRAM(get_reg16(DI), ES), uint8_t(get_reg8(AL) - getRAM(get_reg16(DI), ES)), false);
			if (flags & FLAG_DF) {
				get_reg16(DI)--;
			}
			else {
				get_reg16(DI)++;
			}
			IP += 1;
			break;
		}
			
			//SCASW
		case 0xAF: {
			if (prefRep) {
				do {
					uint16_t mem = (getRAM(get_reg16(DI) + 1, ES) << 8) | getRAM(get_reg16(DI), ES);
					update_flags_arifm(get_reg16(AX), mem, uint16_t(get_reg16(AX) - mem), false);
					if (flags & FLAG_DF) {
						get_reg16(DI) -= 2;
					}
					else {
						get_reg16(DI) += 2;
					}
				} while (get_reg16(CX) && (flags & FLAG_ZF));
				IP += 1;
				prefRep = false;
				break;
			}
			uint16_t mem = (getRAM(get_reg16(DI) + 1, ES) << 8) | getRAM(get_reg16(DI), ES);
			update_flags_arifm(get_reg16(AX), mem, uint16_t(get_reg16(AX) - mem), false);
			if (flags & FLAG_DF) {
				get_reg16(DI) -= 2;
			}
			else {
				get_reg16(DI) += 2;
			}
			IP += 1;
			break;
		}

			//MOV r8 imm8
		case 0xB0:
		case 0xB1:
		case 0xB2:
		case 0xB3:
		case 0xB4:
		case 0xB5:
		case 0xB6:
		case 0xB7: {
			set_r8(opcode - 0xB0, getRAM(IP + 1, CS));
			IP += 2;
			break;
		}
			//MOV r16 imm16
		case 0xB8:
		case 0xB9:
		case 0xBA:
		case 0xBB:
		case 0xBC:
		case 0xBD:
		case 0xBE:
		case 0xBF: {
			get_reg16(opcode - 0xB8) = (getRAM(IP + 2, CS) << 8) | getRAM(IP + 1, CS);
			IP += 3;
			break;
		}
			// групповые операции
		case 0xC0: {
			switch (reg)
			{
				//ROL
			case 0: {
				
				if (mode == 3) {
					for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
						bool highBit = get_reg8(rm) & 0x80;
						set_r8(rm, (get_reg8(rm) << 1) | highBit);
					}
					IP += 3;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < (getRAM(IP + 4, CS) & 0x1F); i++) {
							bool highBit = RAM[addr] & 0x80;
							RAM[addr] = (RAM[addr] << 1) | highBit;
						}
						IP += 5;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
							bool highBit = RAM[addr] & 0x80;
							RAM[addr] = (RAM[addr] << 1) | highBit;
						}
						IP += 3;
						break;
					}
				}

				break;
			}
				  //ROR
			case 1: {
				if (mode == 3) {
					for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
						bool lowBit = get_reg8(rm) & 1;
						set_r8(rm, (get_reg8(rm) >> 1) | (lowBit << 7));
					}
					IP += 3;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < (getRAM(IP + 4, CS) & 0x1F); i++) {
							bool lowBit = RAM[addr] & 1;
							RAM[addr] = (RAM[addr] >> 1) | (lowBit << 7);
						}
						IP += 5;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
							bool lowBit = RAM[addr] & 1;
							RAM[addr] = (RAM[addr] >> 1) | (lowBit << 7);
						}
						IP += 3;
						break;
					}
				}
				break;
			}
				  //RCL
			case 2: {
				if (mode == 3) {
					for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
						bool highBit = get_reg8(rm) & 0x80;
						set_r8(rm, (get_reg8(rm) << 1) | (flags & FLAG_CF));
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
					}
					IP += 3;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < (getRAM(IP + 4, CS) & 0x1F); i++) {
							bool highBit = RAM[addr] & 0x80;
							RAM[addr] = (RAM[addr] << 1) | (flags & FLAG_CF);
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 5;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
							bool highBit = RAM[addr] & 0x80;
							RAM[addr] = (RAM[addr] << 1) | (flags & FLAG_CF);
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 3;
						break;
					}
				}
				break;
			}
				  //RCR
			case 3: {
				if (mode == 3) {
					for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
						bool lowBit = get_reg8(rm) & 1;
						set_r8(rm, (get_reg8(rm) >> 1) | ((flags & FLAG_CF) << 7));
						flags &= ~FLAG_CF;
						flags |= lowBit ? FLAG_CF : 0;
					}
					IP += 3;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < (getRAM(IP + 4, CS) & 0x1F); i++) {
							bool lowBit = RAM[addr] & 1;
							RAM[addr] = (RAM[addr] >> 1) | ((flags & FLAG_CF) << 7);
							flags &= ~FLAG_CF;
							flags |= lowBit ? FLAG_CF : 0;
						}
						IP += 5;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
							bool lowBit = RAM[addr] & 1;
							RAM[addr] = (RAM[addr] >> 1) | ((flags & FLAG_CF) << 7);
							flags &= ~FLAG_CF;
							flags |= lowBit ? FLAG_CF : 0;
						}
						IP += 3;
						break;
					}
				}
				break;
			}
				  //SHL/SAL
			case 4: {
				if (mode == 3) {
					for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
						bool highBit = get_reg8(rm) & 0x80;
						set_r8(rm, (get_reg8(rm) << 1));
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
					}
					IP += 3;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < (getRAM(IP + 4, CS) & 0x1F); i++) {
							bool highBit = RAM[addr] & 0x80;
							RAM[addr] = (RAM[addr] << 1);
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 5;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
							bool highBit = RAM[addr] & 0x80;
							RAM[addr] = (RAM[addr] << 1);
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 3;
						break;
					}
				}
				break;
			}
				  //SHR
			case 5: {
				if (mode == 3) {
					for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
						bool lowBit = get_reg8(rm) & 1;
						set_r8(rm, (get_reg8(rm) >> 1));
						flags &= ~FLAG_CF;
						flags |= lowBit ? FLAG_CF : 0;
					}
					IP += 3;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < (getRAM(IP + 4, CS) & 0x1F); i++) {
							bool lowBit = RAM[addr] & 1;
							RAM[addr] = (RAM[addr] >> 1);
							flags &= ~FLAG_CF;
							flags |= lowBit ? FLAG_CF : 0;
						}
						IP += 5;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
							bool lowBit = RAM[addr] & 1;
							RAM[addr] = (RAM[addr] >> 1);
							flags &= ~FLAG_CF;
							flags |= lowBit ? FLAG_CF : 0;
						}
						IP += 3;
						break;
					}
				}
				break;
			}
				  //SAR
			case 6: {
				if (mode == 3) {
					bool highBit = get_reg8(rm) & 0x80;
					for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
						bool lowBit = get_reg8(rm) & 1;
						set_r8(rm, (get_reg8(rm) >> 1) | (highBit << 7));
						flags &= ~FLAG_CF;

						flags |= lowBit ? FLAG_CF : 0;
					}
					IP += 3;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						bool highBit = RAM[addr] & 0x80;
						for (int i = 0; i < (getRAM(IP + 4, CS) & 0x1F); i++) {
							bool lowBit = RAM[addr] & 1;
							RAM[addr] = (RAM[addr] >> 1) | (highBit << 7);
							flags &= ~FLAG_CF;
							flags |= lowBit ? FLAG_CF : 0;
						}
						IP += 5;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						bool highBit = RAM[addr] & 0x80;
						for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
							bool lowBit = RAM[addr] & 1;
							RAM[addr] = (RAM[addr] >> 1) | (highBit << 7);
							flags &= ~FLAG_CF;
							flags |= lowBit ? FLAG_CF : 0;
						}
						IP += 3;
						break;
					}
				}
				break;
			}
			}
			break;
		}
		case 0xC1: {
			switch (reg)
			{
				//ROL
			case 0: {
				if (mode == 3) {
					for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
						bool highBit = get_reg16(rm) & 0x8000;
						uint16_t r = get_reg16(rm);
						get_reg16(rm) = (r << 1) | highBit;
					}
					IP += 3;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < (getRAM(IP + 4, CS) & 0x1F); i++) {
							bool highBit = RAM[addr+1] & 0x8000;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m <<= 1;
							RAM[addr] = (m & 0xFF) | highBit;
							RAM[addr + 1] = (m >> 8) & 0xFF;
						}
						IP += 5;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
							bool highBit = RAM[addr + 1] & 0x8000;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m <<= 1;
							RAM[addr] = (m & 0xFF) | highBit;
							RAM[addr + 1] = (m >> 8) & 0xFF;
						}
						IP += 3;
						break;
					}
				}
				break;
			}
				  //ROR
			case 1: {
				if (mode == 3) {
					for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
						bool lowBit = get_reg16(rm) & 1;
						uint16_t r = get_reg16(rm);
						get_reg16(rm) = (r >> 1) | (lowBit << 15);
					}
					IP += 3;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < (getRAM(IP + 4, CS) & 0x1F); i++) {
							bool highBit = RAM[addr + 1] & 1;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m >>= 1;
							RAM[addr] = m & 0xFF;
							RAM[addr + 1] = ((m >> 8) & 0xFF) | (highBit << 15);
						}
						IP += 5;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
							bool highBit = RAM[addr + 1] & 1;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m >>= 1;
							RAM[addr] = m & 0xFF;
							RAM[addr + 1] = ((m >> 8) & 0xFF) | (highBit << 15);
						}
						IP += 3;
						break;
					}
				}
				break;
			}
				  //RCL
			case 2: {
				if (mode == 3) {
					for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
						bool highBit = get_reg16(rm) & 0x8000;
						uint16_t r = get_reg16(rm);
						r <<= 1;
						get_reg16(rm) = r | (flags & FLAG_CF);
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
					}
					IP += 3;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < (getRAM(IP + 4, CS) & 0x1F); i++) {
							bool highBit = RAM[addr + 1] & 0x8000;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m <<= 1;
							RAM[addr] = (m & 0xFF) | (flags & FLAG_CF);
							RAM[addr + 1] = (m >> 8) & 0xFF;
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 5;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
							bool highBit = RAM[addr + 1] & 0x8000;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m <<= 1;
							RAM[addr] = (m & 0xFF) | (flags & FLAG_CF);
							RAM[addr + 1] = (m >> 8) & 0xFF;
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 3;
						break;
					}
				}
				break;
			}
				  //RCR
			case 3: {
				if (mode == 3) {
					for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
						bool lowBit = get_reg16(rm) & 1;
						uint16_t r = get_reg16(rm);
						r >>= 1;
						get_reg16(rm) = r | ((flags & FLAG_CF) << 15);
						flags &= ~FLAG_CF;
						flags |= lowBit ? FLAG_CF : 0;
					}
					IP += 3;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < (getRAM(IP + 4, CS) & 0x1F); i++) {
							bool lowBit = RAM[addr + 1] & 1;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m <<= 1;
							RAM[addr] = (m & 0xFF) | ((flags & FLAG_CF) << 15);
							RAM[addr + 1] = (m >> 8) & 0xFF;
							flags &= ~FLAG_CF;
							flags |= lowBit ? FLAG_CF : 0;
						}
						IP += 5;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
							bool lowBit = RAM[addr + 1] & 1;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m <<= 1;
							RAM[addr] = (m & 0xFF) | ((flags & FLAG_CF) << 15);
							RAM[addr + 1] = (m >> 8) & 0xFF;
							flags &= ~FLAG_CF;
							flags |= lowBit ? FLAG_CF : 0;
						}
						IP += 3;
						break;
					}
				}
				break;
			}
				  //SHL/SAL
			case 4: {
				if (mode == 3) {
					for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
						bool highBit = get_reg16(rm) & 0x8000;
						get_reg16(rm) <<= 1;
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
					}
					IP += 3;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < (getRAM(IP + 4, CS) & 0x1F); i++) {
							bool highBit = RAM[addr] & 0x8000;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m <<= 1;
							RAM[addr] = (m & 0xFF);
							RAM[addr + 1] = (m >> 8) & 0xFF;
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 5;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
							bool highBit = RAM[addr] & 0x8000;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m <<= 1;
							RAM[addr] = (m & 0xFF);
							RAM[addr + 1] = (m >> 8) & 0xFF;
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 3;
						break;
					}
				}
				break;
			}
				  //SHR
			case 5: {
				if (mode == 3) {
					for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
						bool highBit = get_reg16(rm) & 1;
						get_reg16(rm) >>= 1;
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
					}
					IP += 3;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < (getRAM(IP + 4, CS) & 0x1F); i++) {
							bool highBit = RAM[addr] & 1;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m >>= 1;
							RAM[addr] = (m & 0xFF);
							RAM[addr + 1] = (m >> 8) & 0xFF;
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 5;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
							bool highBit = RAM[addr] & 1;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m >>= 1;
							RAM[addr] = (m & 0xFF);
							RAM[addr + 1] = (m >> 8) & 0xFF;
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 3;
						break;
					}
				}
				break;
			}
				  //SAR
			case 6: {
				if (mode == 3) {
					bool highBit = get_reg16(rm) & 0x8000;
					for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
						bool lowBit = get_reg16(rm) & 1;
						uint16_t r = get_reg16(rm);
						get_reg16(rm) = (r >> 1) | (highBit << 15);
						flags &= ~FLAG_CF;
						flags |= lowBit ? FLAG_CF : 0;
					}
					IP += 3;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						bool highBit = RAM[addr] & 0x80;
						for (int i = 0; i < (getRAM(IP + 4, CS) & 0x1F); i++) {
							bool lowBit = RAM[addr] & 1;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m >>= 1;
							RAM[addr] = (m & 0xFF);
							RAM[addr + 1] = ((m >> 8) & 0xFF) | (highBit << 7);
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 5;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						bool highBit = RAM[addr] & 0x80;
						for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
							bool lowBit = RAM[addr] & 1;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m >>= 1;
							RAM[addr] = (m & 0xFF);
							RAM[addr + 1] = ((m >> 8) & 0xFF) | (highBit << 7);
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 3;
						break;
					}
				}
				break;
			}
			}
			break;
		}
			//RETN
		case 0xC2: {
			uint16_t offset = (getRAM(IP + 2, CS) << 8) | getRAM(IP + 1, CS);
			IP = (getRAM(get_reg16(SP) + 1, SS) << 8) | getRAM(get_reg16(SP), SS);
			get_reg16(SP) += 2 + offset;
			break;
		}
			//RET
		case 0xC3: {
			IP = (getRAM(get_reg16(SP) + 1, SS) << 8) | getRAM(get_reg16(SP), SS);
			get_reg16(SP) += 2;
			break;
		}
			//LES
		case 0xC4: {
			uint32_t addr = calc_addr(mode, rm);
			get_reg16(reg) = getRAM16(addr);
			segment_registers[ES] = getRAM16(addr);
			IP += 2;
			break;
		}
			//LDS
		case 0xC5: {
			uint32_t addr = calc_addr(mode, rm);
			get_reg16(reg) = getRAM16(addr);
			segment_registers[DS] = getRAM16(addr);
			IP += 2;
			break;
		}
			//MOV rm8 imm8
		case 0xC6: {
			if (mode == 3) {
				set_r8(rm, getRAM(IP+2,CS));
				IP += 3;
				break;
			}
			else {
				if (rm == 6) {
					uint32_t addr = (segment_registers[segment] << 4) | (getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS);
					RAM[addr] = getRAM(IP + 4, CS);
					IP += 5;
					break;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					RAM[addr] = getRAM(IP + 2, CS);
					IP += 3;
					break;
				}
			}
			break;
		}
			//MOV rm16 imm16
		case 0xC7: {
			if (mode == 3) {
				get_reg16(rm) = (getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS);
				IP += 4;
				break;
			}
			else {
				if (rm == 6) {
					uint32_t addr = (segment_registers[segment] << 4) | (getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS);
					RAM[addr] = getRAM(IP + 4, CS);
					RAM[addr + 1] = getRAM(IP + 5, CS);
					IP += 6;
					break;
				}
				else {
					uint32_t addr = calc_addr(mode, rm);
					RAM[addr] = getRAM(IP + 2, CS);
					RAM[addr + 1] = getRAM(IP + 3, CS);
					IP += 4;
					break;
				}
			}
		}
			//RETFN
		case 0xCA: {
			uint16_t imm = (getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS);
			IP = (getRAM(get_reg16(SP) + 1, SS) << 8) | getRAM(get_reg16(SP), SS);
			segment_registers[CS] = (getRAM(get_reg16(SP) + 3, SS) << 8) | getRAM(get_reg16(SP)+2, SS);
			get_reg16(SP) += 4 + imm;
			break;
		}
			//RETF
		case 0xCB: {
			IP = (getRAM(get_reg16(SP) + 1, SS) << 8) | getRAM(get_reg16(SP), SS);
			segment_registers[CS] = (getRAM(get_reg16(SP) + 3, SS) << 8) | getRAM(get_reg16(SP) + 2, SS);
			get_reg16(SP) += 4;
			break;
		}
			//INT 3
		case 0xCC: {
			call_int(3);
			IP += 1;
			break;
		}
			// INT imm8
		case 0xCD: {
			call_int(getRAM(IP + 1, CS));
			IP += 2;
			break;
		}
			//INT if OF
		case 0xCE: {
			if (flags & FLAG_OF) {
				call_int(getRAM(IP + 1, CS));
			}
			IP += 2;
		}
			// RET from INT
		case 0xCF: {
			IP += 1;
		}
			//групповые функции смещения 
		case 0xD0:{
			switch (reg)
			{
				//ROL
			case 0: {

				if (mode == 3) {
					bool highBit = get_reg8(rm) & 0x80;
					set_r8(rm, (get_reg8(rm) << 1) | highBit);
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						bool highBit = RAM[addr] & 0x80;
						RAM[addr] = (RAM[addr] << 1) | highBit;
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						bool highBit = RAM[addr] & 0x80;
						RAM[addr] = (RAM[addr] << 1) | highBit;
						IP += 2;
						break;
					}
				}

				break;
			}
				  //ROR
			case 1: {
				if (mode == 3) {
					bool lowBit = get_reg8(rm) & 1;
					set_r8(rm, (get_reg8(rm) >> 1) | (lowBit << 7));
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						bool lowBit = RAM[addr] & 1;
						RAM[addr] = (RAM[addr] >> 1) | (lowBit << 7);
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						bool lowBit = RAM[addr] & 1;
						RAM[addr] = (RAM[addr] >> 1) | (lowBit << 7);
						IP += 2;
						break;
					}
				}
				break;
			}
				  //RCL
			case 2: {
				if (mode == 3) {
					bool highBit = get_reg8(rm) & 0x80;
					set_r8(rm, (get_reg8(rm) << 1) | (flags & FLAG_CF));
					flags &= ~FLAG_CF;
					flags |= highBit ? FLAG_CF : 0;
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						bool highBit = RAM[addr] & 0x80;
						RAM[addr] = (RAM[addr] << 1) | (flags & FLAG_CF);
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						bool highBit = RAM[addr] & 0x80;
						RAM[addr] = (RAM[addr] << 1) | (flags & FLAG_CF);
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
						IP += 2;
						break;
					}
				}
				break;
			}
				  //RCR
			case 3: {
				if (mode == 3) {
					bool lowBit = get_reg8(rm) & 1;
					set_r8(rm, (get_reg8(rm) >> 1) | ((flags & FLAG_CF) << 7));
					flags &= ~FLAG_CF;
					flags |= lowBit ? FLAG_CF : 0;
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						bool lowBit = RAM[addr] & 1;
						RAM[addr] = (RAM[addr] >> 1) | ((flags & FLAG_CF) << 7);
						flags &= ~FLAG_CF;
						flags |= lowBit ? FLAG_CF : 0;
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						bool lowBit = RAM[addr] & 1;
						RAM[addr] = (RAM[addr] >> 1) | ((flags & FLAG_CF) << 7);
						flags &= ~FLAG_CF;
						flags |= lowBit ? FLAG_CF : 0;
						IP += 2;
						break;
					}
				}
				break;
			}
				  //SHL/SAL
			case 4: {
				if (mode == 3) {
						bool highBit = get_reg8(rm) & 0x80;
						set_r8(rm, (get_reg8(rm) << 1));
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						bool highBit = RAM[addr] & 0x80;
						RAM[addr] = (RAM[addr] << 1);
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						bool highBit = RAM[addr] & 0x80;
						RAM[addr] = (RAM[addr] << 1);
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
						IP += 2;
						break;
					}
				}
				break;
			}
				  //SHR
			case 5: {
				if (mode == 3) {
					bool lowBit = get_reg8(rm) & 1;
					set_r8(rm, (get_reg8(rm) >> 1));
					flags &= ~FLAG_CF;
					flags |= lowBit ? FLAG_CF : 0;
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						bool lowBit = RAM[addr] & 1;
						RAM[addr] = (RAM[addr] >> 1);
						flags &= ~FLAG_CF;
						flags |= lowBit ? FLAG_CF : 0;
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						bool lowBit = RAM[addr] & 1;
						RAM[addr] = (RAM[addr] >> 1);
						flags &= ~FLAG_CF;
						flags |= lowBit ? FLAG_CF : 0;
						IP += 2;
						break;
					}
				}
				break;
			}
				  //SAR
			case 6: {
				if (mode == 3) {
					bool highBit = get_reg8(rm) & 0x80;
					bool lowBit = get_reg8(rm) & 1;
					set_r8(rm, (get_reg8(rm) >> 1) | (highBit << 7));
					flags &= ~FLAG_CF;
					flags |= lowBit ? FLAG_CF : 0;
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						bool highBit = RAM[addr] & 0x80;
						bool lowBit = RAM[addr] & 1;
						RAM[addr] = (RAM[addr] >> 1) | (highBit << 7);
						flags &= ~FLAG_CF;
						flags |= lowBit ? FLAG_CF : 0;
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						bool highBit = RAM[addr] & 0x80;
						bool lowBit = RAM[addr] & 1;
						RAM[addr] = (RAM[addr] >> 1) | (highBit << 7);
						flags &= ~FLAG_CF;
						flags |= lowBit ? FLAG_CF : 0;
						IP += 2;
						break;
					}
				}
				break;
			}
			}
			break;
		}
		case 0xD1: {
			switch (reg)
			{
				//ROL
			case 0: {
				if (mode == 3) {
					bool highBit = get_reg16(rm) & 0x8000;
					uint16_t r = get_reg16(rm);
					get_reg16(rm) = (r << 1) | highBit;
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						bool highBit = RAM[addr + 1] & 0x8000;
						uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
						m <<= 1;
						RAM[addr] = (m & 0xFF) | highBit;
						RAM[addr + 1] = (m >> 8) & 0xFF;
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						bool highBit = RAM[addr + 1] & 0x8000;
						uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
						m <<= 1;
						RAM[addr] = (m & 0xFF) | highBit;
						RAM[addr + 1] = (m >> 8) & 0xFF;
						IP += 2;
						break;
					}
				}
				break;
			}
				  //ROR
			case 1: {
				if (mode == 3) {
					bool lowBit = get_reg16(rm) & 1;
					uint16_t r = get_reg16(rm);
					get_reg16(rm) = (r >> 1) | (lowBit << 15);
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						bool highBit = RAM[addr + 1] & 1;
						uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
						m >>= 1;
						RAM[addr] = m & 0xFF;
						RAM[addr + 1] = ((m >> 8) & 0xFF) | (highBit << 15);
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						bool highBit = RAM[addr + 1] & 1;
						uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
						m >>= 1;
						RAM[addr] = m & 0xFF;
						RAM[addr + 1] = ((m >> 8) & 0xFF) | (highBit << 15);
						IP += 2;
						break;
					}
				}
				break;
			}
				  //RCL
			case 2: {
				if (mode == 3) {
					bool highBit = get_reg16(rm) & 0x8000;
					uint16_t r = get_reg16(rm);
					r <<= 1;
					get_reg16(rm) = r | (flags & FLAG_CF);
					flags &= ~FLAG_CF;
					flags |= highBit ? FLAG_CF : 0;
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						bool highBit = RAM[addr + 1] & 0x8000;
						uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
						m <<= 1;
						RAM[addr] = (m & 0xFF) | (flags & FLAG_CF);
						RAM[addr + 1] = (m >> 8) & 0xFF;
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						bool highBit = RAM[addr + 1] & 0x8000;
						uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
						m <<= 1;
						RAM[addr] = (m & 0xFF) | (flags & FLAG_CF);
						RAM[addr + 1] = (m >> 8) & 0xFF;
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
						IP += 2;
						break;
					}
				}
				break;
			}
				  //RCR
			case 3: {
				if (mode == 3) {
					bool lowBit = get_reg16(rm) & 1;
					uint16_t r = get_reg16(rm);
					r >>= 1;
					get_reg16(rm) = r | ((flags & FLAG_CF) << 15);
					flags &= ~FLAG_CF;
					flags |= lowBit ? FLAG_CF : 0;
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						bool lowBit = RAM[addr + 1] & 1;
						uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
						m <<= 1;
						RAM[addr] = (m & 0xFF) | ((flags & FLAG_CF) << 15);
						RAM[addr + 1] = (m >> 8) & 0xFF;
						flags &= ~FLAG_CF;
						flags |= lowBit ? FLAG_CF : 0;
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						bool lowBit = RAM[addr + 1] & 1;
						uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
						m <<= 1;
						RAM[addr] = (m & 0xFF) | ((flags & FLAG_CF) << 15);
						RAM[addr + 1] = (m >> 8) & 0xFF;
						flags &= ~FLAG_CF;
						flags |= lowBit ? FLAG_CF : 0;
						IP += 2;
						break;
					}
				}
				break;
			}
				  //SHL/SAL
			case 4: {
				if (mode == 3) {
					bool highBit = get_reg16(rm) & 0x8000;
					get_reg16(rm) <<= 1;
					flags &= ~FLAG_CF;
					flags |= highBit ? FLAG_CF : 0;
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						bool highBit = RAM[addr] & 0x8000;
						uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
						m <<= 1;
						RAM[addr] = (m & 0xFF);
						RAM[addr + 1] = (m >> 8) & 0xFF;
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						bool highBit = RAM[addr] & 0x8000;
						uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
						m <<= 1;
						RAM[addr] = (m & 0xFF);
						RAM[addr + 1] = (m >> 8) & 0xFF;
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
						IP += 2;
						break;
					}
				}
				break;
			}
				  //SHR
			case 5: {
				if (mode == 3) {
					bool highBit = get_reg16(rm) & 1;
					get_reg16(rm) >>= 1;
					flags &= ~FLAG_CF;
					flags |= highBit ? FLAG_CF : 0;
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						bool highBit = RAM[addr] & 1;
						uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
						m >>= 1;
						RAM[addr] = (m & 0xFF);
						RAM[addr + 1] = (m >> 8) & 0xFF;
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						bool highBit = RAM[addr] & 1;
						uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
						m >>= 1;
						RAM[addr] = (m & 0xFF);
						RAM[addr + 1] = (m >> 8) & 0xFF;
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
						IP += 2;
						break;
					}
				}
				break;
			}
				  //SAR
			case 6: {
				if (mode == 3) {
					bool highBit = get_reg16(rm) & 0x8000;
					bool lowBit = get_reg16(rm) & 1;
					uint16_t r = get_reg16(rm);
					get_reg16(rm) = (r >> 1) | (highBit << 15);
					flags &= ~FLAG_CF;
					flags |= lowBit ? FLAG_CF : 0;
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						bool highBit = RAM[addr] & 0x80;
						bool lowBit = RAM[addr] & 1;
						uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
						m >>= 1;
						RAM[addr] = (m & 0xFF);
						RAM[addr + 1] = ((m >> 8) & 0xFF) | (highBit << 7);
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						bool highBit = RAM[addr] & 0x80;
						bool lowBit = RAM[addr] & 1;
						uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
						m >>= 1;
						RAM[addr] = (m & 0xFF);
						RAM[addr + 1] = ((m >> 8) & 0xFF) | (highBit << 7);
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
						IP += 2;
						break;
					}
				}
				break;
			}
			}
			break;
		}
		case 0xD2: {
			switch (reg)
			{
				//ROL
			case 0: {

				if (mode == 3) {
					for (int i = 0; i < get_reg8(CL); i++) {
						bool highBit = get_reg8(rm) & 0x80;
						set_r8(rm, (get_reg8(rm) << 1) | highBit);
					}
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < get_reg8(CL); i++) {
							bool highBit = RAM[addr] & 0x80;
							RAM[addr] = (RAM[addr] << 1) | highBit;
						}
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < get_reg8(CL); i++) {
							bool highBit = RAM[addr] & 0x80;
							RAM[addr] = (RAM[addr] << 1) | highBit;
						}
						IP += 2;
						break;
					}
				}

				break;
			}
				  //ROR
			case 1: {
				if (mode == 3) {
					for (int i = 0; i < get_reg8(CL); i++) {
						bool lowBit = get_reg8(rm) & 1;
						set_r8(rm, (get_reg8(rm) >> 1) | (lowBit << 7));
					}
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < get_reg8(CL); i++) {
							bool lowBit = RAM[addr] & 1;
							RAM[addr] = (RAM[addr] >> 1) | (lowBit << 7);
						}
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < get_reg8(CL); i++) {
							bool lowBit = RAM[addr] & 1;
							RAM[addr] = (RAM[addr] >> 1) | (lowBit << 7);
						}
						IP += 2;
						break;
					}
				}
				break;
			}
				  //RCL
			case 2: {
				if (mode == 3) {
					for (int i = 0; i < (getRAM(IP + 2, CS) & 0x1F); i++) {
						bool highBit = get_reg8(rm) & 0x80;
						set_r8(rm, (get_reg8(rm) << 1) | (flags & FLAG_CF));
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
					}
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < get_reg8(CL); i++) {
							bool highBit = RAM[addr] & 0x80;
							RAM[addr] = (RAM[addr] << 1) | (flags & FLAG_CF);
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < get_reg8(CL); i++) {
							bool highBit = RAM[addr] & 0x80;
							RAM[addr] = (RAM[addr] << 1) | (flags & FLAG_CF);
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 2;
						break;
					}
				}
				break;
			}
				  //RCR
			case 3: {
				if (mode == 3) {
					for (int i = 0; i < get_reg8(CL); i++) {
						bool lowBit = get_reg8(rm) & 1;
						set_r8(rm, (get_reg8(rm) >> 1) | ((flags & FLAG_CF) << 7));
						flags &= ~FLAG_CF;
						flags |= lowBit ? FLAG_CF : 0;
					}
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < get_reg8(CL); i++) {
							bool lowBit = RAM[addr] & 1;
							RAM[addr] = (RAM[addr] >> 1) | ((flags & FLAG_CF) << 7);
							flags &= ~FLAG_CF;
							flags |= lowBit ? FLAG_CF : 0;
						}
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < get_reg8(CL); i++) {
							bool lowBit = RAM[addr] & 1;
							RAM[addr] = (RAM[addr] >> 1) | ((flags & FLAG_CF) << 7);
							flags &= ~FLAG_CF;
							flags |= lowBit ? FLAG_CF : 0;
						}
						IP += 2;
						break;
					}
				}
				break;
			}
				  //SHL/SAL
			case 4: {
				if (mode == 3) {
					for (int i = 0; i < get_reg8(CL); i++) {
						bool highBit = get_reg8(rm) & 0x80;
						set_r8(rm, (get_reg8(rm) << 1));
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
					}
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < get_reg8(CL); i++) {
							bool highBit = RAM[addr] & 0x80;
							RAM[addr] = (RAM[addr] << 1);
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < get_reg8(CL); i++) {
							bool highBit = RAM[addr] & 0x80;
							RAM[addr] = (RAM[addr] << 1);
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 2;
						break;
					}
				}
				break;
			}
				  //SHR
			case 5: {
				if (mode == 3) {
					for (int i = 0; i < get_reg8(CL); i++) {
						bool lowBit = get_reg8(rm) & 1;
						set_r8(rm, (get_reg8(rm) >> 1));
						flags &= ~FLAG_CF;
						flags |= lowBit ? FLAG_CF : 0;
					}
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < get_reg8(CL); i++) {
							bool lowBit = RAM[addr] & 1;
							RAM[addr] = (RAM[addr] >> 1);
							flags &= ~FLAG_CF;
							flags |= lowBit ? FLAG_CF : 0;
						}
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < get_reg8(CL); i++) {
							bool lowBit = RAM[addr] & 1;
							RAM[addr] = (RAM[addr] >> 1);
							flags &= ~FLAG_CF;
							flags |= lowBit ? FLAG_CF : 0;
						}
						IP += 2;
						break;
					}
				}
				break;
			}
				  //SAR
			case 6: {
				if (mode == 3) {
					bool highBit = get_reg8(rm) & 0x80;
					for (int i = 0; i < get_reg8(CL); i++) {
						bool lowBit = get_reg8(rm) & 1;
						set_r8(rm, (get_reg8(rm) >> 1) | (highBit << 7));
						flags &= ~FLAG_CF;

						flags |= lowBit ? FLAG_CF : 0;
					}
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						bool highBit = RAM[addr] & 0x80;
						for (int i = 0; i < get_reg8(CL); i++) {
							bool lowBit = RAM[addr] & 1;
							RAM[addr] = (RAM[addr] >> 1) | (highBit << 7);
							flags &= ~FLAG_CF;
							flags |= lowBit ? FLAG_CF : 0;
						}
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						bool highBit = RAM[addr] & 0x80;
						for (int i = 0; i < get_reg8(CL); i++) {
							bool lowBit = RAM[addr] & 1;
							RAM[addr] = (RAM[addr] >> 1) | (highBit << 7);
							flags &= ~FLAG_CF;
							flags |= lowBit ? FLAG_CF : 0;
						}
						IP += 2;
						break;
					}
				}
				break;
			}
			}
			break;
		}
		case 0xD3: {
			switch (reg)
			{
				//ROL
			case 0: {
				if (mode == 3) {
					for (int i = 0; i < get_reg8(CL); i++) {
						bool highBit = get_reg16(rm) & 0x8000;
						uint16_t r = get_reg16(rm);
						get_reg16(rm) = (r << 1) | highBit;
					}
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < get_reg8(CL); i++) {
							bool highBit = RAM[addr + 1] & 0x8000;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m <<= 1;
							RAM[addr] = (m & 0xFF) | highBit;
							RAM[addr + 1] = (m >> 8) & 0xFF;
						}
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < get_reg8(CL); i++) {
							bool highBit = RAM[addr + 1] & 0x8000;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m <<= 1;
							RAM[addr] = (m & 0xFF) | highBit;
							RAM[addr + 1] = (m >> 8) & 0xFF;
						}
						IP += 2;
						break;
					}
				}
				break;
			}
				  //ROR
			case 1: {
				if (mode == 3) {
					for (int i = 0; i < get_reg8(CL); i++) {
						bool lowBit = get_reg16(rm) & 1;
						uint16_t r = get_reg16(rm);
						get_reg16(rm) = (r >> 1) | (lowBit << 15);
					}
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < get_reg8(CL); i++) {
							bool highBit = RAM[addr + 1] & 1;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m >>= 1;
							RAM[addr] = m & 0xFF;
							RAM[addr + 1] = ((m >> 8) & 0xFF) | (highBit << 15);
						}
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < get_reg8(CL); i++) {
							bool highBit = RAM[addr + 1] & 1;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m >>= 1;
							RAM[addr] = m & 0xFF;
							RAM[addr + 1] = ((m >> 8) & 0xFF) | (highBit << 15);
						}
						IP += 2;
						break;
					}
				}
				break;
			}
				  //RCL
			case 2: {
				if (mode == 3) {
					for (int i = 0; i < get_reg8(CL); i++) {
						bool highBit = get_reg16(rm) & 0x8000;
						uint16_t r = get_reg16(rm);
						r <<= 1;
						get_reg16(rm) = r | (flags & FLAG_CF);
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
					}
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < get_reg8(CL); i++) {
							bool highBit = RAM[addr + 1] & 0x8000;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m <<= 1;
							RAM[addr] = (m & 0xFF) | (flags & FLAG_CF);
							RAM[addr + 1] = (m >> 8) & 0xFF;
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < get_reg8(CL); i++) {
							bool highBit = RAM[addr + 1] & 0x8000;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m <<= 1;
							RAM[addr] = (m & 0xFF) | (flags & FLAG_CF);
							RAM[addr + 1] = (m >> 8) & 0xFF;
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 2;
						break;
					}
				}
				break;
			}
				  //RCR
			case 3: {
				if (mode == 3) {
					for (int i = 0; i < get_reg8(CL); i++) {
						bool lowBit = get_reg16(rm) & 1;
						uint16_t r = get_reg16(rm);
						r >>= 1;
						get_reg16(rm) = r | ((flags & FLAG_CF) << 15);
						flags &= ~FLAG_CF;
						flags |= lowBit ? FLAG_CF : 0;
					}
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < get_reg8(CL); i++) {
							bool lowBit = RAM[addr + 1] & 1;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m <<= 1;
							RAM[addr] = (m & 0xFF) | ((flags & FLAG_CF) << 15);
							RAM[addr + 1] = (m >> 8) & 0xFF;
							flags &= ~FLAG_CF;
							flags |= lowBit ? FLAG_CF : 0;
						}
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < get_reg8(CL); i++) {
							bool lowBit = RAM[addr + 1] & 1;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m <<= 1;
							RAM[addr] = (m & 0xFF) | ((flags & FLAG_CF) << 15);
							RAM[addr + 1] = (m >> 8) & 0xFF;
							flags &= ~FLAG_CF;
							flags |= lowBit ? FLAG_CF : 0;
						}
						IP += 2;
						break;
					}
				}
				break;
			}
				  //SHL/SAL
			case 4: {
				if (mode == 3) {
					for (int i = 0; i < get_reg8(CL); i++) {
						bool highBit = get_reg16(rm) & 0x8000;
						get_reg16(rm) <<= 1;
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
					}
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < get_reg8(CL); i++) {
							bool highBit = RAM[addr] & 0x8000;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m <<= 1;
							RAM[addr] = (m & 0xFF);
							RAM[addr + 1] = (m >> 8) & 0xFF;
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < get_reg8(CL); i++) {
							bool highBit = RAM[addr] & 0x8000;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m <<= 1;
							RAM[addr] = (m & 0xFF);
							RAM[addr + 1] = (m >> 8) & 0xFF;
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 2;
						break;
					}
				}
				break;
			}
				  //SHR
			case 5: {
				if (mode == 3) {
					for (int i = 0; i < get_reg8(CL); i++) {
						bool highBit = get_reg16(rm) & 1;
						get_reg16(rm) >>= 1;
						flags &= ~FLAG_CF;
						flags |= highBit ? FLAG_CF : 0;
					}
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						for (int i = 0; i < get_reg8(CL); i++) {
							bool highBit = RAM[addr] & 1;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m >>= 1;
							RAM[addr] = (m & 0xFF);
							RAM[addr + 1] = (m >> 8) & 0xFF;
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						for (int i = 0; i < get_reg8(CL); i++) {
							bool highBit = RAM[addr] & 1;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m >>= 1;
							RAM[addr] = (m & 0xFF);
							RAM[addr + 1] = (m >> 8) & 0xFF;
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 2;
						break;
					}
				}
				break;
			}
				  //SAR
			case 6: {
				if (mode == 3) {
					bool highBit = get_reg16(rm) & 0x8000;
					for (int i = 0; i < get_reg8(CL); i++) {
						bool lowBit = get_reg16(rm) & 1;
						uint16_t r = get_reg16(rm);
						get_reg16(rm) = (r >> 1) | (highBit << 15);
						flags &= ~FLAG_CF;
						flags |= lowBit ? FLAG_CF : 0;
					}
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						bool highBit = RAM[addr] & 0x80;
						for (int i = 0; i < get_reg8(CL); i++) {
							bool lowBit = RAM[addr] & 1;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m >>= 1;
							RAM[addr] = (m & 0xFF);
							RAM[addr + 1] = ((m >> 8) & 0xFF) | (highBit << 7);
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						bool highBit = RAM[addr] & 0x80;
						for (int i = 0; i < get_reg8(CL); i++) {
							bool lowBit = RAM[addr] & 1;
							uint16_t m = (RAM[addr + 1] << 8) | RAM[addr];
							m >>= 1;
							RAM[addr] = (m & 0xFF);
							RAM[addr + 1] = ((m >> 8) & 0xFF) | (highBit << 7);
							flags &= ~FLAG_CF;
							flags |= highBit ? FLAG_CF : 0;
						}
						IP += 2;
						break;
					}
				}
				break;
			}
			}
			break;
		}
			//AAM
		case 0xD4: {
			uint8_t al = get_reg8(AL);
			set_r8(AH, al / 10);
			set_r8(AL, al % 10);
			flags &= ~(FLAG_ZF | FLAG_PF | FLAG_SF);
			if (!get_reg8(AL)) {
				flags |= FLAG_ZF;
			}

			int ones = 0;
			for (int i = 0; i < 8; i++) {
				if (get_reg8(AL) & (1 << i)) {
					ones++;
				}
			}

			flags |= ones % 2 == 0 ? FLAG_PF : 0;

			flags |= (get_reg8(AL) & 0x80) ? FLAG_SF : 0;
			IP += 2;
			break;
		}
			//AAD
		case 0xD5: {
			uint8_t ah = get_reg8(AH);
			set_r8(AL, ah * 10 + get_reg8(AL));
			set_r8(AH, 0);
			flags &= ~(FLAG_ZF | FLAG_PF | FLAG_SF);
			if (!get_reg8(AL)) {
				flags |= FLAG_ZF;
			}

			int ones = 0;
			for (int i = 0; i < 8; i++) {
				if (get_reg8(AL) & (1 << i)) {
					ones++;
				}
			}

			flags |= ones % 2 == 0 ? FLAG_PF : 0;

			flags |= (get_reg8(AL) & 0x80) ? FLAG_SF : 0;
			IP += 2;
			break;
		}
			//SALC
		case 0xD6: {
			if (flags & FLAG_CF)
				set_r8(AL, 0xFF);
			else
				set_r8(AL, 0x00);
			IP += 1;
			break;
		}
			//XLAT
		case 0xD7: {
			set_r8(AL, getRAM(get_reg16(BX) + get_reg8(AL), segment));
			IP += 1;
			break;
		}
			// Для сопроцессора FPU(у меня его к счастью нет)
		case 0xD8:
		case 0xD9:
		case 0xDA:
		case 0xDB:
		case 0xDC:
		case 0xDD:
		case 0xDE:
		case 0xDF: {
			IP += 1;
		}

			//LOOPNE
		case 0xE0: {
			get_reg16(CX)--;
			if (!get_reg16(CX) && !(flags & FLAG_ZF)) {
				IP = getRAM(IP + 1, CS);
			}
			else {
				IP += 2;
			}
			break;
		}

			//LOOPE
		case 0xE1: {
			get_reg16(CX)--;
			if (!get_reg16(CX) && (flags & FLAG_ZF)) {
				IP = getRAM(IP + 1, CS);
			}
			else {
				IP += 2;
			}
			break;
		}

			//LOOP
		case 0xE2: {
			get_reg16(CX)--;
			if (!get_reg16(CX)) {
				IP = getRAM(IP + 1, CS);
			}
			else {
				IP += 2;
			}
			break;
		}

			//JCXZ
		case 0xE3: {
			if (!get_reg16(CX)) {
				IP = getRAM(IP + 1, CS);
			}
			else {
				IP += 2;
			}
			break;
		}

			//IN
		case 0xE4:
		case 0xE5:
		{
			IP += opcode - 0xE2;
			break;
		}
			//OUT
		case 0xE6:
		case 0xE7: {
			IP += opcode - 0xE4;
			break;
		}
			//CALL
		case 0xE8: {
			get_reg16(SP) -= 2;
			getRAM(get_reg16(SP), SS) = IP;
			IP += (getRAM(IP + 2, CS)<<8)|getRAM(IP+1,CS);
			break;
		}
			//JMP
		case 0xE9: {
			IP += (getRAM(IP + 2, CS) << 8) | getRAM(IP + 1, CS);
			break;
		}
		case 0xEA: {
			segment_registers[CS] = (getRAM(IP + 4, CS) << 8) | getRAM(IP + 3, CS);
			IP = (getRAM(IP + 2, CS) << 8) | getRAM(IP + 1, CS);
			break;
		}
		case 0xEB: {
			IP += getRAM(IP + 1, CS);
			break;
		}
			
			//IN
		case 0xEC:
		case 0xED:{
			IP += 1;
			break;
		}
			//OUT
		case 0xEE:
		case 0xEF:{
			IP += 1;
			break;
		}

		case 0xF0: {
			IP += 1;
			break;
		}

		case 0xF1: {
			IP += 1;
			break;
		}
			//REP
		case 0xF2: 
		case 0xF3:{
			prefRep = true;
			IP += 1;
			break;
		}
			//HLT
		case 0xF4: {
			return;
		}
			//CMC
		case 0xF5: {
			bool flag = flags & FLAG_CF;
			flags &= flag ? ~FLAG_CF : FLAG_CF;
			IP += 1;
			break;
		}
			//групповые функции
		case 0xF6: {
			switch (reg)
			{
				//TEST
			case 0: {
				if (mode == 0b11) {
					update_flags_logic(get_reg8(rm) & getRAM(IP+2,CS));
					IP += 3;
				}
				else {
					if (rm == 0b110) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						update_flags_logic(RAM[addr] & getRAM(IP+4,CS));
						IP += 5;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						update_flags_logic(RAM[addr] & getRAM(IP + 2, CS));
						IP += 3;
					}
				}
				break;
			}
				//NOT
			case 2: {
				if (mode == 0b11) {
					set_r8(rm, ~get_reg8(rm));
					IP += 2;
				}
				else {
					if (rm == 0b110) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						RAM[addr] = ~RAM[addr];
						IP += 4;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						RAM[addr] = ~RAM[addr];
						IP += 2;
					}
				}
				break;
			}
				  //NEG
			case 3: {
				if (mode == 0b11) {
					set_r8(rm, 0-get_reg8(rm));
					IP += 2;
				}
				else {
					if (rm == 0b110) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						RAM[addr] = 0-RAM[addr];
						IP += 4;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						RAM[addr] = 0-RAM[addr];
						IP += 2;
					}
				}
				break;
			}
				  //MUL
			case 4: {
				if (mode == 0b11) {
					get_reg16(AX) = get_reg8(AL) * get_reg8(rm);
					IP += 2;
				}
				else {
					if (rm == 6) {
						get_reg16(AX) = get_reg8(AL) * getRAM(getRAM(IP + 2, CS), segment);
						IP += 3;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						get_reg16(AX) = get_reg8(AL) * RAM[addr];
						IP += 2;
					}
				}
				flags &= ~(FLAG_CF | FLAG_OF);
				if (get_reg8(AH)) {
					flags |= FLAG_CF;
				}
				flags |= flags & FLAG_CF ? FLAG_OF : 0;
				break;
			}
				  //IMUL
			case 5: {
				if (mode == 3) {
					int16_t res = int8_t(get_reg8(AL)) * int8_t(get_reg8(rm));
					get_reg16(AX) = res;
					IP += 2;
				}
				else {
					if (rm == 6) {
						int16_t res = int8_t(get_reg8(AL)) * int8_t(getRAM(getRAM(IP + 2, CS), segment));
						get_reg16(AX) = res;
						IP += 3;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						int16_t res = int8_t(get_reg8(AL)) * int8_t(RAM[addr]);
						get_reg16(AX) = res;
						IP += 2;
					}
				}
				flags &= ~(FLAG_CF | FLAG_OF);
				if (get_reg16(AX) < -128 || get_reg16(AX) > 127) {
					flags |= FLAG_CF | FLAG_OF;
				}
				break;
			}
			case 6: {
				if (mode == 3) {
					if (!get_reg8(rm)) return;
					if (get_reg16(AX) / get_reg8(rm) > 255) return;
					uint8_t res = get_reg16(AX) / get_reg8(rm);
					uint8_t remains = get_reg16(AX) % get_reg8(rm);
					set_r8(AL, res);
					set_r8(AH, remains);
					IP += 2;
				}
				else {
					if (rm == 6) {
						if (!getRAM(getRAM(IP + 2, CS), segment)) return;
						if (get_reg16(AX) / getRAM(getRAM(IP + 2, CS), segment) > 255) return;
						uint16_t res = get_reg16(AX) / getRAM(getRAM(IP + 2, CS), segment);
						uint16_t remains = get_reg16(AX) % getRAM(getRAM(IP + 2, CS), segment);
						set_r8(AL, res);
						set_r8(AH, remains);
						IP += 3;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						if (!RAM[addr]) return;
						if (get_reg16(AX) / RAM[addr] > 255) return;
						uint16_t res = get_reg16(AX) / RAM[addr];
						uint16_t remains = get_reg16(AX) % RAM[addr];
						set_r8(AL, res);
						set_r8(AH, remains);
						IP += 2;
					}
				}
				break;
			}
			case 7: {
				if (mode == 3) {
					if (!get_reg8(rm)) return;
					int16_t res = int16_t(get_reg16(AX)) / int8_t(get_reg8(rm));
					int16_t remains = int16_t(get_reg16(AX)) % int8_t(get_reg8(rm));
					set_r8(AL, res);
					set_r8(AH, remains);
					IP += 2;
				}
				else {
					if (rm == 6) {
						if (!getRAM(getRAM(IP + 2, CS), segment)) return;
						int16_t res = (int16_t)get_reg16(AX) / getRAM(getRAM(IP + 2, CS), segment);
						int16_t remains = (int16_t)get_reg16(AX) % getRAM(getRAM(IP + 2, CS), segment);
						set_r8(AL, res);
						set_r8(AH, remains);
						IP += 3;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						if (!RAM[addr]) return;
						int16_t res = (int16_t)get_reg16(AX) / RAM[addr];
						int16_t remains = (int16_t)get_reg16(AX) % RAM[addr];
						set_r8(AL, res);
						set_r8(AH, remains);
						IP += 2;
					}
				}
				break;
			}
			}
			break;
		}
		case 0xF7:  {
			switch (reg)
			{
				//TEST
			case 0: {
				if (mode == 0b11) {
					update_flags_logic(get_reg16(rm) & ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS)));
					IP += 3;
				}
				else {
					if (rm == 0b110) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						update_flags_logic(((RAM[addr + 1] << 8) | RAM[addr]) & ((getRAM(IP + 5, CS) << 8) | getRAM(IP + 4, CS)));
						IP += 6;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						update_flags_logic(((RAM[addr + 1] << 8) | RAM[addr]) & ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS)));
						IP += 4;
					}
				}
				break;
			}
				//NOT
			case 2: {
				if (mode == 0b11) {
					uint16_t neg = ~get_reg16(rm);
					get_reg16(rm) = neg;
					IP += 2;
				}
				else {
					if (rm == 0b110) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						RAM[addr] = ~RAM[addr];
						RAM[addr + 1] = ~RAM[addr + 1];
						IP += 4;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						RAM[addr] = ~RAM[addr];
						RAM[addr + 1] = ~RAM[addr + 1];
						IP += 2;
					}
				}
				break;
			}
				  //NEG
			case 3: {
				if (mode == 0b11) {
					uint16_t neg = get_reg16(rm);
					get_reg16(rm) = 0 - neg;
					IP += 2;
				}
				else {
					if (rm == 0b110) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						RAM[addr] = 0-RAM[addr];
						RAM[addr+1] = 0-RAM[addr+1];
						IP += 4;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						RAM[addr] = 0-RAM[addr];
						RAM[addr+1] = 0-RAM[addr+1];
						IP += 2;
					}
				}
				break;
			}
				  //MUL
			case 4: {
				if (mode == 0b11) {
					uint32_t res = get_reg16(AX) * get_reg16(rm);
					get_reg16(DX) = (res >> 16)&0xFFFF;
					get_reg16(AX) = res & 0xFFFF;
					IP += 2;
				}
				else {
					if (rm == 6) {
						uint32_t res = get_reg16(AX) * getRAM((getRAM(IP + 3, CS) << 8) | getRAM(IP+2,CS), segment);
						get_reg16(DX) = (res >> 16) & 0xFFFF;
						get_reg16(AX) = res & 0xFFFF;
						IP += 4;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						uint32_t res = get_reg8(AX)* ((RAM[addr + 1] << 8) | RAM[addr]);
						get_reg16(DX) = (res >> 16) & 0xFFFF;
						get_reg16(AX) = res & 0xFFFF;
						IP += 2;
					}
				}
				flags &= ~(FLAG_CF | FLAG_OF);
				if (get_reg16(DX)) {
					flags |= FLAG_CF;
				}
				flags |= flags & FLAG_CF ? FLAG_OF : 0;
				break;
			}
				  //IMUL
			case 5: {
				int32_t res = 0;
				if (mode == 3) {
					res = int16_t(get_reg16(AX)) * int16_t(get_reg16(rm));
					get_reg16(DX) = (res >> 16) & 0xFFFF;
					get_reg16(AX) = res & 0xFFFF;
					IP += 2;
				}
				else {
					if (rm == 6) {
						res = int16_t(get_reg16(AX)) * int16_t(getRAM((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS), segment));
						get_reg16(DX) = (res >> 16) & 0xFFFF;
						get_reg16(AX) = res & 0xFFFF;
						IP += 3;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						res = int16_t(get_reg8(AL)) * int16_t(((RAM[addr + 1] << 8) | RAM[addr]));
						get_reg16(DX) = (res >> 16) & 0xFFFF;
						get_reg16(AX) = res & 0xFFFF;
						IP += 2;
					}
				}
				flags &= ~(FLAG_CF | FLAG_OF);
				if (res < -32768 || res > 32767) {
					flags |= FLAG_CF | FLAG_OF;
				}
				break;
			}
				  //DIV
			case 6: {
				if (mode == 3) {
					if (!get_reg16(rm)) return;
					if (((get_reg16(DX) << 8) | (get_reg16(AX))) / get_reg16(rm) > 0xFFFF) return;
					uint16_t res = (get_reg16(DX) << 8) | (get_reg16(AX)) / get_reg16(rm);
					uint16_t remains = (get_reg16(DX) << 8) | (get_reg16(AX)) % get_reg16(rm);
					get_reg16(AX) = res;
					get_reg16(DX) = remains;
					IP += 2;
				}
				else {
					if (rm == 6) {
						if (!getRAM((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS), segment)) return;
						if (((get_reg16(DX) << 8) | (get_reg16(AX))) / getRAM((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS), segment) > 0xFFFF) return;
						uint16_t res = ((get_reg16(DX) << 8) | (get_reg16(AX))) / getRAM((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS), segment);
						uint16_t remains = ((get_reg16(DX) << 8) | (get_reg16(AX))) % getRAM((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS), segment);
						get_reg16(AX) = res;
						get_reg16(DX) = remains;
						IP += 4;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						if (!((RAM[addr + 1] << 8) | RAM[addr])) return;
						if ((((get_reg16(DX) << 8) | (get_reg16(AX))) / ((RAM[addr + 1] << 8) | RAM[addr])) > 0xFFFF) return;
						uint16_t res = get_reg16(AX) / ((RAM[addr + 1] << 8) | RAM[addr]);
						uint16_t remains = get_reg16(AX) % ((RAM[addr + 1] << 8) | RAM[addr]);
						get_reg16(AX) = res;
						get_reg16(DX) = remains;
						IP += 2;
					}
				}
				break;
			}
			case 7: {
				if (mode == 3) {
					if (!get_reg16(rm)) return;
					if (((get_reg16(DX) << 8) | (get_reg16(AX))) / get_reg16(rm) > 0xFFFF) return;
					int16_t res = int16_t(get_reg16(DX) << 8) | (get_reg16(AX)) / int8_t(get_reg16(rm));
					int16_t remains = int16_t(get_reg16(DX) << 8) | (get_reg16(AX)) % int8_t(get_reg16(rm));
					get_reg16(AX) = res;
					get_reg16(DX) = remains;
					IP += 2;
				}
				else {
					if (rm == 6) {
						if (!getRAM((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS), segment)) return;
						if (((get_reg16(DX) << 8) | (get_reg16(AX))) / getRAM((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS), segment) > 0xFFFF) return;
						int16_t res = (int16_t)((get_reg16(DX) << 8) | (get_reg16(AX))) / int16_t(getRAM((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS), segment));
						int16_t remains = (int16_t)((get_reg16(DX) << 8) | (get_reg16(AX))) % int16_t(getRAM((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS), segment));
						get_reg16(AX) = res;
						get_reg16(DX) = remains;
						IP += 4;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						if (!((RAM[addr + 1] << 8) | RAM[addr])) return;
						if ((((get_reg16(DX) << 8) | (get_reg16(AX))) / ((RAM[addr + 1] << 8) | RAM[addr])) > 0xFFFF) return;
						int16_t res = (int16_t)get_reg16(AX) / int16_t((RAM[addr + 1] << 8) | RAM[addr]);
						int16_t remains = (int16_t)get_reg16(AX) % int16_t((RAM[addr + 1] << 8) | RAM[addr]);
						get_reg16(AX) = res;
						get_reg16(DX) = remains;
						IP += 2;
					}
				}
				break;
			}
			}
			break;
		}
			//CLC
		case 0xF8: {
			flags &= ~FLAG_CF;
			IP += 1;
			break;
		}
			//STC
		case 0xF9: {
			flags |= FLAG_CF;
			IP += 1;
			break;
		}
			//CLI
		case 0xFA: {
			flags &= ~FLAG_IF;
			IP += 1;
			break;
		}
			//STI
		case 0xFB: {
			flags |= FLAG_IF;
			IP += 1;
			break;
		}
			//CLD
		case 0xFC: {
			flags &= ~FLAG_DF;
			IP += 1;
			break;
		}
			//STD
		case 0xFD: {
			flags |= FLAG_DF;
			IP += 1;
			break;
		}
			//INC DEC rm8	 
		case 0xFE: {
			//INC
			if (reg == 0) {
				if (mode == 3) {
					set_r8(rm, get_reg8(rm) + 1);
					update_flags_inc(get_reg8(rm), true);
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						RAM[addr]++;
						update_flags_inc(RAM[addr], true);
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						RAM[addr]++;
						update_flags_inc(RAM[addr], true);
						IP += 2;
						break;
					}
				}
			}
			//DEC
			else if(reg == 1){
				if (mode == 3) {
					set_r8(rm, get_reg8(rm) - 1);
					update_flags_inc(get_reg8(rm), false);
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						RAM[addr]--;
						update_flags_inc(RAM[addr], false);
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						RAM[addr]--;
						update_flags_inc(RAM[addr], false);
						IP += 2;
						break;
					}
				}
			}
			else {
				std::cout << "Unknown opcode" << '\n';
				IP += 1;
				break;
			}
		}
			//INC DEC rm16
		case 0xFF: {
			if (reg == 0) {
				if (mode == 3) {
					get_reg16(rm) = get_reg16(rm)++;
					update_flags_inc(get_reg16(rm), false);
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr];
						res++;
						RAM[addr] = res & 0xFF;
						RAM[addr+1] = (res >> 8) & 0xFF;
						update_flags_inc((RAM[addr + 1] << 8) | RAM[addr], true);
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr];
						res++;
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						update_flags_inc((RAM[addr + 1] << 8) | RAM[addr], true);
						IP += 2;
						break;
					}
				}
			}
			//DEC
			else if (reg == 1) {
				if (mode == 3) {
					get_reg16(rm) = get_reg16(rm)--;
					update_flags_inc(get_reg16(rm), false);
					IP += 2;
					break;
				}
				else {
					if (rm == 6) {
						uint32_t addr = (segment_registers[segment] << 4) | ((getRAM(IP + 3, CS) << 8) | getRAM(IP + 2, CS));
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr];
						res--;
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						update_flags_inc((RAM[addr + 1] << 8) | RAM[addr], true);
						IP += 4;
						break;
					}
					else {
						uint32_t addr = calc_addr(mode, rm);
						uint16_t res = (RAM[addr + 1] << 8) | RAM[addr];
						res--;
						RAM[addr] = res & 0xFF;
						RAM[addr + 1] = (res >> 8) & 0xFF;
						update_flags_inc((RAM[addr + 1] << 8) | RAM[addr], true);
						IP += 2;
						break;
					}
				}
			}
			else {
				std::cout << "Unknown opcode" << '\n';
				IP += 1;
				break;
			}
		}
		}

	}
}

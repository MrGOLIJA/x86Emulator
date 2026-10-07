#pragma once
#include <iostream>
#include <stdlib.h>

//флаги
#define FLAG_CF   (1 << 0)  
#define FLAG_PF   (1 << 2)  
#define FLAG_AF   (1 << 4)  
#define FLAG_ZF   (1 << 6)  
#define FLAG_SF   (1 << 7)  
#define FLAG_TF   (1 << 8)  
#define FLAG_IF   (1 << 9)  
#define FLAG_DF   (1 << 10) 
#define FLAG_OF   (1 << 11) 

//16-битные регситры
#define AX 0
#define CX 1
#define DX 2
#define BX 3
#define SP 4
#define BP 5
#define SI 6
#define DI 7

//8-битные регистры
#define AL 0
#define CL 1
#define DL 2
#define BL 3
#define AH 4
#define CH 5
#define DH 6
#define BH 7

//сегментные регистры
#define ES 0
#define CS 1
#define SS 2
#define DS 3

#define getRAM(offset, seg) RAM[(segment_registers[seg] << 4) | (offset)]
#define getRAM16(addr) (RAM[(addr)+1]<<8) | RAM[(addr)]


class cpu_8088
{
public:
	cpu_8088();
	~cpu_8088();

	void emulate();
	void loadFileToRAM(FILE* file);

private:
	uint16_t registers[8];
	uint16_t segment_registers[4];

	uint8_t get_reg8(int index);
	uint16_t& get_reg16(int index);

	uint32_t calc_addr(int mode, int rm);

	void call_int(int num_int);

	void print_all_reg();

	template<typename T>
	void update_flags_arifm(T a, T b, T res, bool isAdd) {
		flags &= ~FLAG_CF | ~FLAG_OF | ~FLAG_ZF | ~FLAG_SF | ~FLAG_PF | ~FLAG_AF;

		flags |= res == 0 ? FLAG_ZF : 0;
		if (isAdd) {
			flags |= a + b > (1<<sizeof(T)*8)-1 ? FLAG_CF : 0;
		}
		else {
			flags |= a < b ? FLAG_CF : 0;
		}
		flags |= (res >> (sizeof(T)*8 - 1)) & 1 ? FLAG_SF : 0;

		if (((a << (sizeof(T)*8)-1) & 1) == ((b << (sizeof(T) * 8) -1) & 1) && ((res << (sizeof(T) * 8) -1) & 1) == ((a << (sizeof(T) * 8) -1) & 1)) {
			flags |= FLAG_OF;
		}

		uint8_t low_byte = res & 0xFF;
		int ones = 0;
		for (int i = 0; i < 8; i++) {
			if (low_byte & (1 << i)) {
				ones++;
			}
		}

		flags |= ones % 2 == 0 ? FLAG_PF : 0;

		if ((a & 0x0F) + (b & 0x0F) > 0x0F) {
			flags |= FLAG_AF;
		}
	}
	
	template<typename T>
	void update_flags_logic(T res) {
		flags &= ~(FLAG_CF | FLAG_OF | FLAG_SF | FLAG_ZF | FLAG_PF | FLAG_AF);

		flags |= (res >> (sizeof(T)*8-1)) & 1 ? FLAG_SF : 0;

		flags |= res == 0 ? FLAG_ZF : 0;

		uint8_t low_byte = res & 0xFF;
		int ones = 0;
		for (int i = 0; i < 8; i++) {
			if (low_byte & (1 << i)) {
				ones++;
			}
		}

		flags |= ones % 2 == 0 ? FLAG_PF : 0;
	}

	template<typename T>
	void update_flags_inc(T res, bool inc) {
		flags &= ~(FLAG_ZF | FLAG_PF | FLAG_SF | FLAG_AF | FLAG_OF);

		flags |= res == 0 ? FLAG_ZF : 0;
		flags |= res >> ((sizeof(T) * 8) - 1) ? FLAG_SF : 0;

		uint8_t low_byte = res & 0xFF;
		int ones = 0;
		for (int i = 0; i < 8; i++) {
			if (low_byte & (1 << i)) {
				ones++;
			}
		}

		flags |= ones % 2 == 0 ? FLAG_PF : 0;


		if (inc) {
			flags |= ((res - 1) & 0x0F) == 0x0F ? FLAG_AF : 0;
			if constexpr (sizeof(T) == 1) {
				flags |= (res - 1) == 0x7F ? FLAG_OF : 0;
			}
			else if constexpr (sizeof(T) == 2){
				flags |= (res - 1) == 0x7FFF ? FLAG_OF : 0;
			}
		}
		else {
			flags |= ((res + 1) &0x0F) == 0x00 ? FLAG_AF : 0;
			if constexpr (sizeof(T) == 1) {
				flags |= (res + 1) == 0x80 ? FLAG_OF : 0;
			}
			else if constexpr (sizeof(T) == 2) {
				flags |= (res + 1) == 0x8000 ? FLAG_OF : 0;
			}
		}
	}

	void set_r8(int index, uint8_t value);

	uint16_t IP = 0;
	uint16_t flags = 0;

	uint8_t* RAM;

	bool prefRep = false;

	bool changeSeg = false;

	int segment = DS;
};


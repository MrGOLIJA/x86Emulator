#define _CRT_SECURE_NO_WARNINGS
#include "cpu_8088.h"
#include <windows.h>

int main() {
	cpu_8088 cpu;
	FILE* file = fopen("prog.com","r");
	cpu.loadFileToRAM(file);
	cpu.emulate();
	return 0;
}
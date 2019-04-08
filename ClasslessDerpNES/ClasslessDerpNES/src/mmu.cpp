#include "mmu.h"
#include "ppu.h"
#include "controller.h"
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

//Memory
uint8_t main_memory[65535];
uint8_t graphics_memory[65535];
//Rom stuff
uint8_t romInfo[16];
uint8_t Vmirror;

Mapper * mapper;

uint8_t getVmirror() { return Vmirror; }

void m_write(uint16_t loc, uint8_t data)
{
	switch (loc) {
	case 0x2000: setRegFlag(0); break;
	case 0x2001: setRegFlag(1); break;
	case 0x2002: setRegFlag(2); break;
	case 0x2003: setRegFlag(3); break;
	case 0x2004: setRegFlag(4); break;
	case 0x2005: setRegFlag(5); break;
	case 0x2006: setRegFlag(6); break;
	case 0x2007: setRegFlag(7); break;
	case 0x4014: setRegFlag(8); break;
	default:
		break;
	}

	if (loc >= 0x800 && loc < 0x2000) { cout << hex << "Write At: " << (int)loc << endl; system("PAUSE"); }
	if (loc >= 0x2008 && loc < 0x4000) { cout << hex << "Write At: " << (int)loc << endl; system("PAUSE"); }

	if (loc == 0x4016) { return controller::lockInputs(data); }

	if (loc >= 0x8000) 
	{
		mapper->update(loc, data);
	}
	else 
	{
		main_memory[loc] = data;
	}
}

uint8_t m_read(uint16_t loc)
{
	if (loc >= 0x800 && loc < 0x2000) { cout << hex << "Read At: " << (int)loc << endl; system("PAUSE"); }
	if (loc >= 0x2008 && loc < 0x4000) { cout << hex << "Read At: " << (int)loc << endl; system("PAUSE"); }

	if (loc == 0x4016) { return controller::readController1(); }

	//std::cout << hex << (int)loc << std::endl;
	//std::cout << hex << (int)main_memory[loc] << std::endl;
	//system("pause");

	return main_memory[loc];
}

void g_write(uint16_t loc, uint8_t data)
{
	/*if (loc >= 0x2000 && loc < 0x2400) 
	{
		if (Vmirror == 1) { graphics_memory[loc] = data; graphics_memory[loc + 0x800]; }
		else { graphics_memory[loc] = data; graphics_memory[loc + 0x400]; }
		return;
	}
	else if (loc >= 0x2400 && loc < 0x2800)
	{
		if (Vmirror == 1) { graphics_memory[loc] = data; graphics_memory[loc + 0x800]; }
		else { graphics_memory[loc] = data; graphics_memory[loc + 0x400]; }
		return;
	}
	else if (loc >= 0x2800 && loc < 0x2C00)
	{
		if (Vmirror == 1) { graphics_memory[loc] = data; graphics_memory[loc + 0x800]; }
		else { graphics_memory[loc] = data; graphics_memory[loc + 0x400]; }
		return;
	}
	else if (loc >= 0x2C00 && loc < 0x3000)
	{
		if (Vmirror == 1) { graphics_memory[loc] = data; graphics_memory[loc + 0x800]; }
		else { graphics_memory[loc] = data; graphics_memory[loc + 0x400]; }
		return;
	}*/
	//mirrors
	if (loc >= 0x3000 && loc < 0x3F00) { graphics_memory[loc - 0x1000] = data; }
	else if (loc >= 0x3F20) { std::cout << "High graphics adress write called (Mirrors are not set up to handle this adress properly)"; system("pause"); }
	else {
		graphics_memory[loc] = data;
	}
}

uint8_t g_read(uint16_t loc)
{
	if (loc >= 0x3000 && loc < 0x3F00) { return graphics_memory[loc - 0x1000]; }
	else if (loc >= 0x3F20) {std::cout << "High graphics adress read called (Mirrors are not set up to handle this adress properly): "<< std::hex << (int)loc << std::endl;system("pause");}
	return graphics_memory[loc];
}

void m_NoCheck_write(uint16_t loc, uint8_t data)
{
	main_memory[loc] = data;
}

void loadRom(std::string file)
{
	std::ifstream infile;
	//Load rom
	infile.open(file, std::ios::binary | std::ios::in);
	if (!infile) {
		std::cout << "Error: Can't open the file named: " << file << std::endl;
		system("PAUSE");
	}
	//read info bytes
	for (int i = 0; i < 16; i++) {
		romInfo[i] = infile.get();
	}

	switch ((romInfo[6] >> 4) == 1) 
	{
	case 0:
		mapper = new MM0();
		break;
	case 1:
		mapper = new MM1();
		break;
	}

	mapper->setUp(&infile);

	Vmirror = romInfo[6] & 0x1;
}

uint8_t* getRomInfo() 
{
	return romInfo;
}

void Load2_8Kb_Banks() 
{

}

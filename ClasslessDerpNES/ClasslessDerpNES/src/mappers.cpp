#include "mmu.h"
#include <fstream>
#include <iostream>

using namespace std;

//General Mapper ---------------------------------------------------------------------------------------------------------------------------

void Mapper::load_PRG_bank_to_memory(uint8_t src, uint8_t dist)
{
	switch (dist) 
	{
	case 0:
		for (int i = 0; i < 0x4000; i++) {
			m_NoCheck_write(0x8000 + i, PRG_rom_banks[src][i]);
		}
		break;
	case 1:
		for (int i = 0; i < 0x4000; i++) {
			m_NoCheck_write(0xC000 + i, PRG_rom_banks[src][i]);
		}
		break;
	default:
		std::cout << "WRIGHT TO PRG BANK " << (int)dist << " DETECTED, INVALID LOCATION" << std::endl;
		//system("PAUSE");
		break;
	}
}

void Mapper::load_CHR_bank_to_memory(uint8_t src, uint8_t dist)
{
	switch (dist)
	{
	case 0:
		for (int i = 0; i < 0x1000; i++) {
			g_write(0x0000 + i, CHR_rom_banks[src][i]);
		}
		break;
	case 1:
		for (int i = 0; i < 0x1000; i++) {
			g_write(0x1000 + i, CHR_rom_banks[src][i]);
		}
		break;
	default:
		std::cout << "WRIGHT TO CHR BANK " << (int)dist << " DETECTED, INVALID LOCATION" << std::endl;
		//system("PAUSE");
		break;
	}
}

//MM0 --------------------------------------------------------------------------------------------------------------------------------------

void MM0::update(uint16_t loc, uint8_t data)
{
	//MM0 doesn't change ram banks
}

void MM0::setUp(std::ifstream* stream)
{
	uint8_t* rom = getRomInfo();
	//16Kb PRG rom
	if (rom[4] == 1) 
	{
		for (int i = 0; i < 0x4000; i++) {
			uint8_t hold = stream->get();
			PRG_rom_banks[0][i] = hold;
			//PRG_rom_banks[1][i] = hold;
		}

		load_PRG_bank_to_memory(0, 0);
		load_PRG_bank_to_memory(0, 1);

		for (int i = 0; i < 0x1000; i++) {
			CHR_rom_banks[0][i] = stream->get();
		}
		for (int i = 0; i < 0x1000; i++) {
			CHR_rom_banks[1][i] = stream->get();
		}

		load_CHR_bank_to_memory(0, 0);
		load_CHR_bank_to_memory(1, 1);
	}
	//32Kb PRG rom
	else 
	{
		for (int i = 0; i < 0x4000; i++) {
			PRG_rom_banks[0][i] = stream->get();
		}
		for (int i = 0; i < 0x4000; i++) {
			PRG_rom_banks[1][i] = stream->get();
		}

		load_PRG_bank_to_memory(0, 0);
		load_PRG_bank_to_memory(1, 1);

		for (int i = 0; i < 0x1000; i++) {
			CHR_rom_banks[0][i] = stream->get();
		}
		for (int i = 0; i < 0x1000; i++) {
			CHR_rom_banks[1][i] = stream->get();
		}

		load_CHR_bank_to_memory(0, 0);
		load_CHR_bank_to_memory(1, 1);
	}
}

//MM1 --------------------------------------------------------------------------------------------------------------------------------------

void MM1::update(uint16_t loc, uint8_t data)
{
	//std::cout << std::hex << "Loc: " << (int)loc  << " Data: " << (int)data << std::endl;

	if ((data >> 7) == 1)
	{
		//reset
	}
	else
	{
		//Take A bit
		if (dataCounter < 4)
		{
			LoadReg = LoadReg | ((data & 0x1) << 4);
			LoadReg = (LoadReg >> 1);
			dataCounter++;
		}
		//Loads register to the proper one
		else
		{
			LoadReg = LoadReg | ((data & 0x1) << 4);
			dataCounter = 0;

			//std::cout << std::hex << "Fin: " << (int)LoadReg << std::endl;

			//copy register
			switch ((loc & 0x6000) >> 13) {
			case 0:
				for (int i = 0; i < 5; i++) { ControlReg = LoadReg; }
				break;
			case 1:
				for (int i = 0; i < 5; i++) { CHR_Bank_0_Reg = LoadReg; }
				break;
			case 2:
				for (int i = 0; i < 5; i++) { CHR_Bank_1_Reg = LoadReg; }
				break;
			case 3:
				for (int i = 0; i < 5; i++) { PRG_Bank_Reg = LoadReg; }
				break;
			}
			//Reset Load register
			LoadReg = 0;
		}
		
	}

	//Do what registers say
	if ((ControlReg >> 4) == 1) 
	{
		load_CHR_bank_to_memory(CHR_Bank_0_Reg, 0);
		load_CHR_bank_to_memory(CHR_Bank_1_Reg, 1);
	}
	if (((ControlReg >> 2) & 0x3) == 3)
	{
		load_PRG_bank_to_memory(PRG_Bank_Reg, 0);
	}
	

	//cout << "control Reg: " << hex << (int)ControlReg << endl;
	//system("pause");
}

void MM1::setUp(std::ifstream* stream)
{
	uint8_t* rom = getRomInfo();

	for (int d = 0; d < rom[4]; d++) 
	{
		for (int i = 0; i < 0x4000; i++)
		{
			PRG_rom_banks[d][i] = stream->get();
		}
	}

	for (int d = 0; d < rom[5] * 2; d+=2)
	{
		for (int i = 0; i < 0x1000; i++)
		{
			CHR_rom_banks[d][i] = stream->get();
		}
		for (int i = 0; i < 0x1000; i++)
		{
			CHR_rom_banks[d+1][i] = stream->get();
		}
	}

	load_PRG_bank_to_memory(14, 0);
	load_PRG_bank_to_memory(15, 1);

	load_CHR_bank_to_memory(0, 0);
	load_CHR_bank_to_memory(1, 1);
}

//MM2 --------------------------------------------------------------------------------------------------------------------------------------

void MM2::setUp(std::ifstream * stream)
{
	uint8_t* rom = getRomInfo();

	for (int d = 0; d < rom[4]; d++)
	{
		for (int i = 0; i < 0x4000; i++)
		{
			PRG_rom_banks[d][i] = stream->get();
		}
	}

	for (int d = 0; d < rom[5] * 2; d += 2)
	{
		for (int i = 0; i < 0x1000; i++)
		{
			CHR_rom_banks[d][i] = stream->get();
		}
		for (int i = 0; i < 0x1000; i++)
		{
			CHR_rom_banks[d + 1][i] = stream->get();
		}
	}

	load_PRG_bank_to_memory(0, 0);
	load_PRG_bank_to_memory(7, 1);

	load_CHR_bank_to_memory(0, 0);
	load_CHR_bank_to_memory(1, 1);
}

void MM2::update(uint16_t loc, uint8_t data)
{
	reg = data;
	load_PRG_bank_to_memory(reg, 0);
}
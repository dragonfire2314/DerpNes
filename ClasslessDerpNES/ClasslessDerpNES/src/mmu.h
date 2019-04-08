#pragma once
#include <stdint.h>
#include <string>
#include <vector>

void m_write(uint16_t loc, uint8_t data);
uint8_t m_read(uint16_t loc);
void g_write(uint16_t loc, uint8_t data);
uint8_t g_read(uint16_t loc);
void m_NoCheck_write(uint16_t loc, uint8_t data);

uint8_t* getRomInfo();

uint8_t getVmirror();

void loadRom(std::string file);

//Mappers Classes
class Mapper
{
private:
public:

	struct Rom_bank 
	{
		uint8_t bank[0x4000];
	};

	Rom_bank romBank;

	//Creates 16 16kb banks of PRG memory
	//uint8_t PRG_rom_banks[16][0x4000];
	//Creates 32 4kb banks of CHR memory
	uint8_t CHR_rom_banks[32][0x1000];

	std::vector<Rom_bank> PRG_rom_banks;

	virtual void update(uint16_t loc, uint8_t data) = 0;
	virtual void setUp(std::ifstream* stream) = 0;

	void load_PRG_bank_to_memory(uint8_t src, uint8_t dist);
	//src - is the which bank 1-32 you want to move //dist is either ram slot 0 or 1
	void load_CHR_bank_to_memory(uint8_t src, uint8_t dist);
};

class MM0 : public Mapper
{
	void update(uint16_t loc, uint8_t data) override;
	void setUp(std::ifstream* stream) override;
};

class MM1 : public Mapper
{
private:
	/*uint8_t LoadReg[5];
	uint8_t ControlReg[5];
	uint8_t CHR_Bank_0_Reg[5];
	uint8_t CHR_Bank_1_Reg[5];
	uint8_t PRG_Bank_Reg[5];*/
	uint8_t LoadReg = 0;
	uint8_t ControlReg = 0;
	uint8_t CHR_Bank_0_Reg = 0;
	uint8_t CHR_Bank_1_Reg = 0;
	uint8_t PRG_Bank_Reg = 0;
	int dataCounter = 0;

public:
	void update(uint16_t loc, uint8_t data) override;
	void setUp(std::ifstream* stream) override;
};
#pragma once
#include <stdint.h>

namespace apu 
{
	void audioSetUp();
	void tick();
	void UpdateReg(uint16_t loc, uint8_t data);
}
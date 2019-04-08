#include "controller.h"
#include <iostream>

namespace controller {

	uint8_t inputs[8];
	uint8_t inputs2[8];
	uint8_t counter = 0;

	void lockInputs(uint8_t data)
	{
		if (data == 1)
		{
			for (int i = 0; i < 8; i++) {
				inputs2[i] = inputs[i];
			}
		}
	}

	uint8_t readController1()
	{
		uint8_t hold = (inputs2[counter] & 0x1);
		counter++;
		if (counter == 8) { counter = 0; }
		//std::cout << std::hex << "T: " << (int)hold << std::endl;
		//system("pause");
		return hold | 0x40;
	}

	void setButtonPress(Button b, uint8_t isOn)
	{
		inputs[b] = (int)isOn;

		std::cout << (int)inputs[0]
			<< (int)inputs[1]
			<< (int)inputs[2]
			<< (int)inputs[3]
			<< (int)inputs[4]
			<< (int)inputs[5]
			<< (int)inputs[6]
			<< (int)inputs[7]
			<< std::endl;
	}

}
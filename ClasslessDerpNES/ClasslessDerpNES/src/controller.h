#pragma once
#include <stdint.h>

enum Button {A=0, B, SELECT, START, UP, DOWN, LEFT, RIGHT};

namespace controller {
	void lockInputs(uint8_t data);
	uint8_t readController1();
	void setButtonPress(Button b, uint8_t isOn);

}
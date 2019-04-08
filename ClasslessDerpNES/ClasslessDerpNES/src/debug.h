#pragma once
#include <stdint.h>

void Debug_createMemBanksView();
void Debug_Update();
void Debug_DrawPixel(uint8_t x, uint8_t y, uint8_t col, uint8_t palette, char type);
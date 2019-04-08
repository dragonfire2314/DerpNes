#pragma once
#include <stdint.h>
#include <SDL.h>

void checkFlags();
void startVblank();
void endVblank();
void run();
void DrawSprites();
void DrawPixel(uint16_t x, uint16_t y, uint8_t col, uint8_t palette, char type);
void DrawPixel_s(uint16_t x, uint16_t y, uint8_t col, uint8_t palette, char type);
void DrawBackground();
void setRegFlag(uint8_t reg);
unsigned char* GetDebugPixels();

uint8_t PPU_getBackgroundLayerLoc();

unsigned char* GetPixelsPonter();
SDL_Rect* getRect();
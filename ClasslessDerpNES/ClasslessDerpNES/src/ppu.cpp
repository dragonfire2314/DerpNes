#include "ppu.h"
#include "mmu.h"
#include "main.h"
#include <iostream>

//texture Size
const int texWidth = 512;
const int texHeight = 240;
//screen of pixels
unsigned char pixels[256 * 240 * 4] = { 0 };
//Nametable screen
uint8_t pixel_nametable[512 * 480 * 4];
//color pallet
uint8_t b_colorlookup[0x3F] =
{
	0x7C,
	0x00,
	0x00,
	0x44,
	0x94,
	0xA8,
	0xA8,
	0x88,
	0x50,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xBC,
	0x00,
	0x00,
	0x68,
	0xD8,
	0xE4,
	0xF8,
	0xE4,
	0xAC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xF8,
	0x3C,
	0x68,
	0x98,
	0xF8,
	0xF8,
	0xF8,
	0xFC,
	0xF8,
	0xB8,
	0x58,
	0x58,
	0x00,
	0x78,
	0x00,
	0x00,
	0xFC,
	0xA4,
	0xB8,
	0xD8,
	0xF8,
	0xF8,
	0xF0,
	0xFC,
	0xF8,
	0xD8,
	0xB8,
	0xB8,
	0x00,
	0xF8,
	0x00
};
uint8_t g_colorlookup[0x3F] =
{
	0x7C,
	0x00,
	0x00,
	0x28,
	0x00,
	0x00,
	0x10,
	0x14,
	0x30,
	0x78,
	0x68,
	0x58,
	0x40,
	0x00,
	0x00,
	0x00,
	0xBC,
	0x78,
	0x58,
	0x44,
	0x00,
	0x00,
	0x38,
	0x5C,
	0x7C,
	0xB8,
	0xA8,
	0xA8,
	0x88,
	0x00,
	0x00,
	0x00,
	0xF8,
	0xBC,
	0x88,
	0x78,
	0x78,
	0x58,
	0x78,
	0xA0,
	0xB8,
	0xF8,
	0xD8,
	0xF8,
	0xE8,
	0x78,
	0x00,
	0x00,
	0xFC,
	0xE4,
	0xB8,
	0xB8,
	0xB8,
	0xA4,
	0xD0,
	0xE0,
	0xD8,
	0xF8,
	0xF8,
	0xF8,
	0xFC,
	0xD8,
	0x00,
};
uint8_t r_colorlookup[0x3F] =
{
	0x7C,
	0xFC,
	0xBC,
	0xBC,
	0x84,
	0x20,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x58,
	0x00,
	0x00,
	0x00,
	0xBC,
	0xF8,
	0xF8,
	0xFC,
	0xCC,
	0x58,
	0x00,
	0x10,
	0x00,
	0x00,
	0x00,
	0x44,
	0x88,
	0x00,
	0x00,
	0x00,
	0xF8,
	0xFC,
	0xFC,
	0xF8,
	0xF8,
	0x98,
	0x58,
	0x44,
	0x00,
	0x18,
	0x54,
	0x98,
	0xD8,
	0x78,
	0x00,
	0x00,
	0xFC,
	0xFC,
	0xF8,
	0xF8,
	0xF8,
	0xC0,
	0xB0,
	0xA8,
	0x78,
	0x78,
	0xB8,
	0xD8,
	0xFC,
	0xF8,
	0x00
};
//SDL Veiw Rect
SDL_Rect rect;
//Register Flags
bool regFlags[9];
//VARS STUFF
uint8_t baseNameTable;
uint8_t spritePatternTable;
uint8_t backgroundPatternTable;
uint8_t spriteSzie;
uint8_t generateNMI;

uint8_t vBlank = 0;

uint8_t lesft_8_pixels_show_background;
uint8_t lesft_8_pixels_show_sprites;
uint8_t show_background;
uint8_t show_sprites;

uint16_t dataOffset = 0;

uint8_t ppuaddr_low;
uint8_t ppuaddr_high;
bool ishigh = true;

bool ScrollHigh = true;

uint8_t oam[64 * 4] = { 0 };
uint8_t oamAddr = 0;

int xScroll = 0;
int yScroll = 0;

unsigned char* GetDebugPixels() 
{
	return pixel_nametable;
}

void setRegFlag(uint8_t reg)
{
	regFlags[reg] = true;
}

void startVblank()
{
	
	m_NoCheck_write(0x2002, m_read(0x2002) | 0x80);
	if (generateNMI)
	{
		nmi();
	}
}

void endVblank()
{
	m_NoCheck_write(0x2002, m_read(0x2002) ^ 0x80);
}

void run()
{
	//Render Background
	if (show_background == 1)
	{
		DrawBackground();
	}
	//Render Sprites if shown
	if (show_sprites == 1)
	{
		DrawSprites();
	}
}

void checkFlags()
{
	//0x2000
	if (regFlags[0])
	{
		uint8_t hold = m_read(0x2000);

		baseNameTable = hold & 0x3;

		spritePatternTable = (hold >> 3) & 0x1;
		backgroundPatternTable = (hold >> 4) & 0x1;
		generateNMI = ((hold >> 7) & 0x1);
	}
	//0x2001
	if (regFlags[1])
	{
		uint8_t hold = m_read(0x2001);
		if ((hold & 0x02)) { lesft_8_pixels_show_background = 1; }
		else { lesft_8_pixels_show_background = 0; }
		if ((hold & 0x04)) { lesft_8_pixels_show_sprites = 1; }
		else { lesft_8_pixels_show_sprites = 0; }
		if ((hold & 0x08)) { show_background = 1; }
		else { show_background = 0; }
		if ((hold & 0x10)) { show_sprites = 1; }
		else { show_sprites = 0; }
		//STOP("B: ", hold);
	}
	//0x2002 Write only
	if (regFlags[2])
	{

	}
	//0x2003
	if (regFlags[3])
	{
		oamAddr = m_read(0x2003);
	}
	if (regFlags[4])
	{
		oam[oamAddr] = m_read(0x2004);
		oamAddr++;
	}
	//0x2005
	if (regFlags[5])
	{
		if (ScrollHigh)
		{
			xScroll = m_read(0x2005);
			ScrollHigh = false;
		}
		else
		{
			yScroll = m_read(0x2005);
			ScrollHigh = true;
		}
	}
	//0x2006
	if (regFlags[6])
	{
		dataOffset = 0;

		if (ishigh)
		{
			ppuaddr_high = m_read(0x2006);
			ishigh = false;
		}
		else
		{
			ppuaddr_low = m_read(0x2006);
			ishigh = true;
		}
	}
	//0x2007
	if (regFlags[7])
	{
		if (getVmirror() == 1) { }
		else { 
			//g_write(((ppuaddr_high << 8) + (ppuaddr_low)) + dataOffset, m_read(0x2007));
			g_write(((ppuaddr_high << 8) + (ppuaddr_low)) + dataOffset, m_read(0x2007));
		}

		//g_write(((ppuaddr_high << 8) + (ppuaddr_low)) + dataOffset, m_read(0x2007));
		dataOffset++;
	}
	//0x4014
	if (regFlags[8])
	{
		//Should halt processor but currently does not do that
		uint8_t hold = m_read(0x4014);

		for (int i = 0; i < 256; i++)
		{
			oam[i] = m_read((0x0100*hold) + i);
		}
	}

	//Reset flags
	for (int i = 0; i < 9; i++) { regFlags[i] = false; }

	//write back flags that need read privillage
	//Hopefully it works
	m_NoCheck_write(0x2004, oam[oamAddr]);
}

void DrawSprites()
{
	//8x8 sprites
	//Looks though OAM, should do it in revese for proper emulation since sprite 0 is drawn last.
	for (int b = 0; b < 64; b++)
	{
		uint8_t b0, b1, b2, b3, temp2, temp3, temp4;
		b0 = oam[0x0 + b * 4] + 1;//Ypos
		b1 = oam[0x1 + b * 4];//Index
		b2 = oam[0x2 + b * 4];//Attributes
		b3 = oam[0x3 + b * 4];//Xpos

		temp2 = b2 & 0x03;
		temp3 = (b2 >> 7) & 0x1;
		temp4 = (b2 >> 6) & 0x1;

		if (temp4 == 1 && temp3 == 1) {
			//Draws sprite
			int e = 7;
			for (int k = 0; k < 8; k++) {

				uint8_t c1, c2;
				uint8_t p1[8], p2[8];
				c1 = g_read(0x0000 + e + (b1 * 0x10) + (0x1000 * spritePatternTable));
				c2 = g_read(0x0008 + e + (b1 * 0x10) + (0x1000 * spritePatternTable));

				p1[0] = (c1 >> 7) & 1; p2[0] = (c2 >> 7) & 1;
				p1[1] = (c1 >> 6) & 1; p2[1] = (c2 >> 6) & 1;
				p1[2] = (c1 >> 5) & 1; p2[2] = (c2 >> 5) & 1;
				p1[3] = (c1 >> 4) & 1; p2[3] = (c2 >> 4) & 1;
				p1[4] = (c1 >> 3) & 1; p2[4] = (c2 >> 3) & 1;
				p1[5] = (c1 >> 2) & 1; p2[5] = (c2 >> 2) & 1;
				p1[6] = (c1 >> 1) & 1; p2[6] = (c2 >> 1) & 1;
				p1[7] = (c1 >> 0) & 1; p2[7] = (c2 >> 0) & 1;

				int f = 7;
				for (int i = 0; i < 8; i++)
				{
					DrawPixel_s(i + b3, k + b0, (p2[f] << 1) + p1[f], temp2, 1);
					f--;
				}
				e--;
			}
		}
		else if(temp4 == 1) {
			//Draws sprite
			for (int k = 0; k < 8; k++) {

				uint8_t c1, c2;
				uint8_t p1[8], p2[8];
				c1 = g_read(0x0000 + k + (b1 * 0x10) + (0x1000 * spritePatternTable));
				c2 = g_read(0x0008 + k + (b1 * 0x10) + (0x1000 * spritePatternTable));

				p1[0] = (c1 >> 7) & 1; p2[0] = (c2 >> 7) & 1;
				p1[1] = (c1 >> 6) & 1; p2[1] = (c2 >> 6) & 1;
				p1[2] = (c1 >> 5) & 1; p2[2] = (c2 >> 5) & 1;
				p1[3] = (c1 >> 4) & 1; p2[3] = (c2 >> 4) & 1;
				p1[4] = (c1 >> 3) & 1; p2[4] = (c2 >> 3) & 1;
				p1[5] = (c1 >> 2) & 1; p2[5] = (c2 >> 2) & 1;
				p1[6] = (c1 >> 1) & 1; p2[6] = (c2 >> 1) & 1;
				p1[7] = (c1 >> 0) & 1; p2[7] = (c2 >> 0) & 1;

				int f = 7;
				for (int i = 0; i < 8; i++)
				{
					DrawPixel_s(i + b3, k + b0, (p2[f] << 1) + p1[f], temp2, 1);
					f--;
				}
			}
		}
		else if (temp3 == 1) {
			//Draws sprite
			int e = 7;
			for (int k = 0; k < 8; k++) {

				uint8_t c1, c2;
				uint8_t p1[8], p2[8];
				c1 = g_read(0x0000 + e + (b1 * 0x10) + (0x1000 * spritePatternTable));
				c2 = g_read(0x0008 + e + (b1 * 0x10) + (0x1000 * spritePatternTable));

				p1[0] = (c1 >> 7) & 1; p2[0] = (c2 >> 7) & 1;
				p1[1] = (c1 >> 6) & 1; p2[1] = (c2 >> 6) & 1;
				p1[2] = (c1 >> 5) & 1; p2[2] = (c2 >> 5) & 1;
				p1[3] = (c1 >> 4) & 1; p2[3] = (c2 >> 4) & 1;
				p1[4] = (c1 >> 3) & 1; p2[4] = (c2 >> 3) & 1;
				p1[5] = (c1 >> 2) & 1; p2[5] = (c2 >> 2) & 1;
				p1[6] = (c1 >> 1) & 1; p2[6] = (c2 >> 1) & 1;
				p1[7] = (c1 >> 0) & 1; p2[7] = (c2 >> 0) & 1;

				for (int i = 0; i < 8; i++)
				{
					DrawPixel_s(i + b3, k + b0, (p2[i] << 1) + p1[i], temp2, 1);
				}
				e--;
			}
		}
		else {
			//Draws sprite
			for (int k = 0; k < 8; k++) {

				uint8_t c1, c2;
				uint8_t p1[8], p2[8];
				c1 = g_read(0x0000 + k + (b1 * 0x10) + (0x1000 * spritePatternTable));
				c2 = g_read(0x0008 + k + (b1 * 0x10) + (0x1000 * spritePatternTable));

				p1[0] = (c1 >> 7) & 1; p2[0] = (c2 >> 7) & 1;
				p1[1] = (c1 >> 6) & 1; p2[1] = (c2 >> 6) & 1;
				p1[2] = (c1 >> 5) & 1; p2[2] = (c2 >> 5) & 1;
				p1[3] = (c1 >> 4) & 1; p2[3] = (c2 >> 4) & 1;
				p1[4] = (c1 >> 3) & 1; p2[4] = (c2 >> 3) & 1;
				p1[5] = (c1 >> 2) & 1; p2[5] = (c2 >> 2) & 1;
				p1[6] = (c1 >> 1) & 1; p2[6] = (c2 >> 1) & 1;
				p1[7] = (c1 >> 0) & 1; p2[7] = (c2 >> 0) & 1;

				for (int i = 0; i < 8; i++)
				{
					DrawPixel_s(i + b3, k + b0, (p2[i] << 1) + p1[i], temp2, 1);
				}
			}
		}
	}
}

void DrawBackground()
{
	for (int v = 0; v < 4; v++)
	{
		for (int b = 0; b < 960; b++) {
			uint8_t temp, temp2, x1, y1, x2, y2, x4, y4, u1, u2, u3, e1;
			temp = g_read(0x2000 + b + (v * 0x400));
			x1 = b % 32;
			y1 = b / 32;
			x2 = x1 / 2;
			y2 = y1 / 2;
			y4 = y1 / 4;
			x4 = x1 / 4;
			u1 = b / 4;
			temp2 = g_read(0x23C0 + (y4 * 8) + x4 + (v * 0x400));

			e1 = (y1 * 8) + x1;

			//Top Row
			if ((y2 % 2) == 0) {
				//Left
				if ((x2 % 2) == 0) {
					temp2 = (temp2 >> 0) & 0x03;
				}
				//Right
				else {
					temp2 = (temp2 >> 2) & 0x03;
				}
			}
			//Bottom Row
			else {
				//Left
				if ((x2 % 2) == 0) {
					temp2 = (temp2 >> 4) & 0x03;
				}
				//Right
				else {
					temp2 = (temp2 >> 6) & 0x03;
				}
			}

			for (int k = 0; k < 8; k++) {

				uint8_t c1, c2;
				uint8_t p1[8], p2[8];
				c1 = g_read(0x0000 + k + (temp * 0x10) + (0x1000 * backgroundPatternTable));
				c2 = g_read(0x0008 + k + (temp * 0x10) + (0x1000 * backgroundPatternTable));

				p1[0] = (c1 >> 7) & 1; p2[0] = (c2 >> 7) & 1;
				p1[1] = (c1 >> 6) & 1; p2[1] = (c2 >> 6) & 1;
				p1[2] = (c1 >> 5) & 1; p2[2] = (c2 >> 5) & 1;
				p1[3] = (c1 >> 4) & 1; p2[3] = (c2 >> 4) & 1;
				p1[4] = (c1 >> 3) & 1; p2[4] = (c2 >> 3) & 1;
				p1[5] = (c1 >> 2) & 1; p2[5] = (c2 >> 2) & 1;
				p1[6] = (c1 >> 1) & 1; p2[6] = (c2 >> 1) & 1;
				p1[7] = (c1 >> 0) & 1; p2[7] = (c2 >> 0) & 1;

				for (int i = 0; i < 8; i++)
				{
					DrawPixel(i + ((b % 32) * 8) + ((v % 2) * 256), k + ((b / 32) * 8) + ((v / 2) * 240), (p2[i] << 1) + p1[i], temp2, 0);
				}
			}
		}
	}

	//Copy pixels
	for (int i = 0; i < 256 * 240 * 4; i++)
	{
		int tempX, tempY;
		tempY = (i / 1024);
		tempX = (i % 1024);
		//point in nametable array
		int x1 = tempX + xScroll + (baseNameTable % 2) * 256;
		int y1 = tempY + yScroll + (baseNameTable / 2) * 240;

		if (y1 >= 480) {
			pixels[i] = pixel_nametable[(((tempY + yScroll + (baseNameTable / 2) * 240) - 480) * 2048) + tempX + (xScroll*4) + (baseNameTable % 2) * 1024];
		}
		else if (x1 >= 512 * 4) {
			pixels[i] = pixel_nametable[(((tempY + yScroll + (baseNameTable / 2) * 240)) * 2048) + (tempX + (xScroll * 4) + (baseNameTable % 2) * 1024) - 2048];
		}
		else if (x1 >= 512 && y1 >= 480) {
			pixels[i] = pixel_nametable[(((tempY + yScroll + (baseNameTable / 2) * 240) - 480) * 2048) + (tempX + (xScroll * 4) + (baseNameTable % 2) * 1024) - 2048];
		}
		else 
		{
			pixels[i] = pixel_nametable[((tempY + yScroll + (baseNameTable / 2) * 240) * 2048) + tempX + (xScroll*4) + (baseNameTable % 2) * 1024];
		}
	}
}

void DrawPixel(uint16_t x, uint16_t y, uint8_t col, uint8_t palette, char type)
{

	uint8_t temp = g_read(0x3F00 + (4 * palette) + col);

	if (col == 1)
	{
		pixel_nametable[(((texWidth*y) + x) * 4) + 0] = b_colorlookup[temp];//b
		pixel_nametable[(((texWidth*y) + x) * 4) + 1] = g_colorlookup[temp];//g
		pixel_nametable[(((texWidth*y) + x) * 4) + 2] = r_colorlookup[temp];//r
		pixel_nametable[(((texWidth*y) + x) * 4) + 3] = 255;//a
	}
	else if (col == 2)
	{
		pixel_nametable[(((texWidth*y) + x) * 4) + 0] = b_colorlookup[temp];//b
		pixel_nametable[(((texWidth*y) + x) * 4) + 1] = g_colorlookup[temp];//g
		pixel_nametable[(((texWidth*y) + x) * 4) + 2] = r_colorlookup[temp];//r
		pixel_nametable[(((texWidth*y) + x) * 4) + 3] = 255;//a
	}
	else if (col == 3)
	{
		pixel_nametable[(((texWidth*y) + x) * 4) + 0] = b_colorlookup[temp];//b
		pixel_nametable[(((texWidth*y) + x) * 4) + 1] = g_colorlookup[temp];//g
		pixel_nametable[(((texWidth*y) + x) * 4) + 2] = r_colorlookup[temp];//r
		pixel_nametable[(((texWidth*y) + x) * 4) + 3] = 255;//a
	}
	else
	{
		pixel_nametable[(((texWidth*y) + x) * 4) + 0] = b_colorlookup[g_read(0x3F00)];//b
		pixel_nametable[(((texWidth*y) + x) * 4) + 1] = g_colorlookup[g_read(0x3F00)];//g
		pixel_nametable[(((texWidth*y) + x) * 4) + 2] = r_colorlookup[g_read(0x3F00)];//r
		pixel_nametable[(((texWidth*y) + x) * 4) + 3] = 255;//a
	}
}

uint8_t PPU_getBackgroundLayerLoc() 
{
	return backgroundPatternTable;
}

unsigned char* GetPixelsPonter() 
{
	return pixels;
}

SDL_Rect* getRect() 
{
	rect.h = 240;
	rect.w = 256;

	rect.x = xScroll;
	rect.y = 300;

	//std::cout << xScroll << std::endl;
	std::cout << yScroll << std::endl;

	return &rect;
}

void DrawPixel_s(uint16_t x, uint16_t y, uint8_t col, uint8_t palette, char type)
{
	uint8_t temp = g_read(0x3F10 + (4 * palette) + col);

	if (col == 1)
	{
		pixels[(((256 *y) + x) * 4) + 0] = b_colorlookup[temp];//b
		pixels[(((256 *y) + x) * 4) + 1] = g_colorlookup[temp];//g
		pixels[(((256 *y) + x) * 4) + 2] = r_colorlookup[temp];//r
		pixels[(((256 *y) + x) * 4) + 3] = 255;//a
	}
	else if (col == 2)
	{
		pixels[(((256 *y) + x) * 4) + 0] = b_colorlookup[temp];//b
		pixels[(((256 *y) + x) * 4) + 1] = g_colorlookup[temp];//g
		pixels[(((256 *y) + x) * 4) + 2] = r_colorlookup[temp];//r
		pixels[(((256 *y) + x) * 4) + 3] = 255;//a
	}
	else if (col == 3)
	{
		pixels[(((256 *y) + x) * 4) + 0] = b_colorlookup[temp];//b
		pixels[(((256 *y) + x) * 4) + 1] = g_colorlookup[temp];//g
		pixels[(((256 *y) + x) * 4) + 2] = r_colorlookup[temp];//r
		pixels[(((256 *y) + x) * 4) + 3] = 255;//a
	}
}
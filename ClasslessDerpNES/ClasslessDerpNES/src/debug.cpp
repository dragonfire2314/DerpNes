#include "debug.h"
#include "mmu.h"
#include "ppu.h"
#include <SDL.h>

void Debug_update_nametables();


uint8_t Debug_pixels[256 * 240 * 4 * 4];

uint8_t Debug_b_colorlookup[0x3F] =
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
uint8_t Debug_g_colorlookup[0x3F] =
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
uint8_t Debug_r_colorlookup[0x3F] =
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

SDL_Window *Debug_window;
SDL_Renderer *Debug_renderer;
SDL_Texture *Debug_texture;

SDL_Window *Debug_window_2;
SDL_Renderer *Debug_renderer_2;
SDL_Texture *Debug_texture_2;

void Debug_createMemBanksView() 
{
	Debug_window = SDL_CreateWindow("DEBUG", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 256, SDL_WINDOW_SHOWN);
	Debug_renderer = SDL_CreateRenderer(Debug_window, -1, SDL_RENDERER_ACCELERATED);
	Debug_texture = SDL_CreateTexture(Debug_renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 256, 128);

	Debug_window_2 = SDL_CreateWindow("DEBUG_NAMETABLES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 480, SDL_WINDOW_SHOWN);
	Debug_renderer_2 = SDL_CreateRenderer(Debug_window_2, -1, SDL_RENDERER_ACCELERATED);
	Debug_texture_2 = SDL_CreateTexture(Debug_renderer_2, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 512, 480);
}

void Debug_Update() 
{
	//cls
	SDL_SetRenderDrawColor(Debug_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(Debug_renderer);

	SDL_SetRenderDrawColor(Debug_renderer_2, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(Debug_renderer_2);

	for (int q = 0; q < 256; q++) {
		for (int k = 0; k < 8; k++) {

			uint8_t c1, c2;
			uint8_t p1[8], p2[8];
			c1 = g_read(0x0000 + k + (0x10 * q));
			c2 = g_read(0x0008 + k + (0x10 * q));

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
				Debug_DrawPixel(i + ((q % 16) * 8), k + ((q / 16) * 8), (p2[i] << 1) + p1[i], 0, 0);
			}
		}
	}

	for (int q = 0; q < 256; q++) {
		for (int k = 0; k < 8; k++) {

			uint8_t c1, c2;
			uint8_t p1[8], p2[8];
			c1 = g_read(0x1000 + k + (0x10 * q));
			c2 = g_read(0x1008 + k + (0x10 * q));

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
				Debug_DrawPixel(i + ((q % 16) * 8) + 128, k + ((q / 16) * 8), (p2[i] << 1) + p1[i], 0, 0);
			}
		}
	}

	Debug_update_nametables();

	//update frame
	SDL_UpdateTexture(Debug_texture, NULL, Debug_pixels, 512 * 4);
	SDL_RenderCopy(Debug_renderer, Debug_texture, NULL, NULL);
	SDL_RenderPresent(Debug_renderer);

	SDL_UpdateTexture(Debug_texture_2, NULL, GetDebugPixels(), 512 * 4);
	SDL_RenderCopy(Debug_renderer_2, Debug_texture_2, NULL, NULL);
	SDL_RenderPresent(Debug_renderer_2);
}

void Debug_update_nametables() 
{
	
}

void Debug_DrawPixel(uint8_t x, uint8_t y, uint8_t col, uint8_t palette, char type)
{
	if (type == 0)
	{
		uint8_t temp = g_read(0x3F00 + (4 * palette) + col);

		if (col == 1)
		{
			Debug_pixels[(((512*y) + x) * 4) + 0] = Debug_b_colorlookup[temp];//b
			Debug_pixels[(((512*y) + x) * 4) + 1] = Debug_g_colorlookup[temp];//g
			Debug_pixels[(((512*y) + x) * 4) + 2] = Debug_r_colorlookup[temp];//r
			Debug_pixels[(((512*y) + x) * 4) + 3] = 255;//a
		}
		else if (col == 2)
		{
			Debug_pixels[(((512*y) + x) * 4) + 0] = Debug_b_colorlookup[temp];//b
			Debug_pixels[(((512*y) + x) * 4) + 1] = Debug_g_colorlookup[temp];//g
			Debug_pixels[(((512*y) + x) * 4) + 2] = Debug_r_colorlookup[temp];//r
			Debug_pixels[(((512*y) + x) * 4) + 3] = 255;//a
		}
		else if (col == 3)
		{
			Debug_pixels[(((512*y) + x) * 4) + 0] = Debug_b_colorlookup[temp];//b
			Debug_pixels[(((512*y) + x) * 4) + 1] = Debug_g_colorlookup[temp];//g
			Debug_pixels[(((512*y) + x) * 4) + 2] = Debug_r_colorlookup[temp];//r
			Debug_pixels[(((512*y) + x) * 4) + 3] = 255;//a
		}
		else
		{
			Debug_pixels[(((512*y) + x) * 4) + 0] = Debug_b_colorlookup[g_read(0x3F00)];//b
			Debug_pixels[(((512*y) + x) * 4) + 1] = Debug_g_colorlookup[g_read(0x3F00)];//g
			Debug_pixels[(((512*y) + x) * 4) + 2] = Debug_r_colorlookup[g_read(0x3F00)];//r
			Debug_pixels[(((512*y) + x) * 4) + 3] = 255;//a
		}
	}
	else
	{
		uint8_t temp = g_read(0x3F10 + (4 * palette) + col);

		if (col == 1)
		{
			Debug_pixels[(((512*y) + x) * 4) + 0] = Debug_b_colorlookup[temp];//b
			Debug_pixels[(((512*y) + x) * 4) + 1] = Debug_g_colorlookup[temp];//g
			Debug_pixels[(((512*y) + x) * 4) + 2] = Debug_r_colorlookup[temp];//r
			Debug_pixels[(((512*y) + x) * 4) + 3] = 255;//a
		}
		else if (col == 2)
		{
			Debug_pixels[(((512*y) + x) * 4) + 0] = Debug_b_colorlookup[temp];//b
			Debug_pixels[(((512*y) + x) * 4) + 1] = Debug_g_colorlookup[temp];//g
			Debug_pixels[(((512*y) + x) * 4) + 2] = Debug_r_colorlookup[temp];//r
			Debug_pixels[(((512*y) + x) * 4) + 3] = 255;//a
		}
		else if (col == 3)
		{
			Debug_pixels[(((512*y) + x) * 4) + 0] = Debug_b_colorlookup[temp];//b
			Debug_pixels[(((512*y) + x) * 4) + 1] = Debug_g_colorlookup[temp];//g
			Debug_pixels[(((512*y) + x) * 4) + 2] = Debug_r_colorlookup[temp];//r
			Debug_pixels[(((512*y) + x) * 4) + 3] = 255;//a
		}
	}
}


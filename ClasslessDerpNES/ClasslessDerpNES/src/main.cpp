#include <SDL.h>
#include <thread>

#include "mos6502.h"
#include "mmu.h"
#include "ppu.h"
#include "main.h"
#include "debug.h"
#include "controller.h"
#include "menu.h"

//functions
void check_SDLevents();

//events
SDL_Event e;
bool running = true;
bool callNMI = false;

int main(int argc, char *argv[])
{
	std::thread t(menu::guiRun);

	loadRom("marioW.nes");

	mos6502 mos(m_read, m_write);

	//SDL
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window *window = SDL_CreateWindow("DerpNES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 480, SDL_WINDOW_SHOWN);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);

	Debug_createMemBanksView();

	SDL_Rect rect;
	rect.h = 240;
	rect.w = 256;

	//Main loop
	while (running)
	{
		const Uint64 start = SDL_GetPerformanceCounter();

		Debug_Update();

		//cls
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
		//Check events
		check_SDLevents();

		//Do frame calcs
		//Run cpu for 241 scanlines (241*341)
		for (int i = 0; i < 82181; i++)
		{
			mos.Run(1);
			checkFlags();
		}
		//set v blank on ppu
		startVblank();
		if (callNMI) { mos.NMI(); callNMI = false; }
		//Run cpu for 19 scanlines (19*341)
		for (int i = 0; i < 6479; i++)
		{
			mos.Run(1);
			checkFlags();
		}
		//End Vblank
		endVblank();
		//Render the screen
		run();

		//update frame
		SDL_UpdateTexture(texture, NULL, GetPixelsPonter(), 256 * 4);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);

		//Calculate frame times
		const Uint64 end = SDL_GetPerformanceCounter();
		const static Uint64 freq = SDL_GetPerformanceFrequency();
		const double seconds = (end - start) / static_cast< double >(freq);
		//cout << "Frame time: " << seconds * 1000.0 << "ms" << endl;
	}
	//Clean up
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_DestroyTexture(texture);
	SDL_Quit();

	return 0;
}

void check_SDLevents()
{
	while (SDL_PollEvent(&e))
	{
		if ((SDL_QUIT == e.type) ||
			(SDL_KEYDOWN == e.type && SDL_SCANCODE_ESCAPE == e.key.keysym.scancode))
		{
			running = false;
			break;
		}
		if (e.type == SDL_KEYDOWN) 
		{
			switch (e.key.keysym.sym) 
			{
			case SDLK_UP:
				controller::setButtonPress(UP, 1);
				break;
			case SDLK_DOWN:
				controller::setButtonPress(DOWN, 1);
				break;
			case SDLK_LEFT:
				controller::setButtonPress(LEFT, 1);
				break;
			case SDLK_RIGHT:
				controller::setButtonPress(RIGHT, 1);
				break;
			case SDLK_z:
				controller::setButtonPress(A, 1);
				break;
			case SDLK_x:
				controller::setButtonPress(B, 1);
				break;
			case SDLK_RETURN:
				controller::setButtonPress(START, 1);
				break;
			case SDLK_BACKSPACE:
				controller::setButtonPress(SELECT, 1);
				break;
			}
		}
		if (e.type == SDL_KEYUP)
		{
			switch (e.key.keysym.sym)
			{
			case SDLK_UP:
				controller::setButtonPress(UP, 0);
				break;
			case SDLK_DOWN:
				controller::setButtonPress(DOWN, 0);
				break;
			case SDLK_LEFT:
				controller::setButtonPress(LEFT, 0);
				break;
			case SDLK_RIGHT:
				controller::setButtonPress(RIGHT, 0);
				break;
			case SDLK_z:
				controller::setButtonPress(A, 0);
				break;
			case SDLK_x:
				controller::setButtonPress(B, 0);
				break;
			case SDLK_RETURN:
				controller::setButtonPress(START, 0);
				break;
			case SDLK_BACKSPACE:
				controller::setButtonPress(SELECT, 0);
				break;
			}
		}
	}
}

void nmi() 
{
	callNMI = true;
}
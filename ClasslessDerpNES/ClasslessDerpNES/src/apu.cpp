#include "apu.h"
#include "main.h"

#include <SDL.h>
#include <math.h>

#include <iostream>
using namespace std;

#define TAU 6.28318530718
#define PI 3.14159265359
#define CPU 1789773

namespace apu
{
	//length timer look up table
	uint8_t APU_lookup_table[0x20]
	{
		//0x0
		10, 254, 20, 2,
		40, 4, 80, 6,
		160, 8, 60, 10,
		14, 12, 36, 14,
		//0x10
		12, 16, 24, 18,
		48, 20, 96, 22,
		192, 24, 72, 26,
		16, 28, 32, 30
	};

	struct Pusle
	{
		//Byte 1
		uint8_t duty = 0;
		bool halt = false;
		bool constVolume = false;
		uint8_t envelope = 0;
		//Byte 2
		bool sweepUnitEnable = false;
		uint8_t period = 0;
		bool negate = false;
		uint8_t shift = 0;
		//Byte 3 + 4
		uint16_t timer = 0;
		uint8_t timerH = 0;
		uint8_t timerL = 0;
		uint8_t length = 0;

		//Other Stuff
		bool isLow = 1;
		double frequency = 0;

		uint8_t real_length = 0;
	};

	struct Triangle 
	{
		uint16_t timer = 0;
		uint8_t timerH = 0;
		uint8_t timerL = 0;

		double frequency = 0;
	};

	struct Noise 
	{
		uint8_t length = 0;
		uint8_t noisePeriod;
		bool isNoiseLoop;
	};

	struct AudioData
	{
		double T = 0;
		int Ocatave = 2;
		double volume = 0.1;

		double GTime = 0.0;
		double TimeStep = 1.0 / 48000;

		bool enableP1 = false;
		bool enableP2 = false;
		bool enableTri = false;
		bool enableNoise = false;
		bool enableDMC = false;

		bool mode0;
		bool blockIRQ;

		void tick(uint8_t counter) 
		{
			switch (counter) 
			{
			case 0:
				break;
			case 1:
				if (P1.length > 0) { P1.real_length--; }
				if (P2.length > 0) { P2.real_length--; }
				break;
			case 2:
				break;
			case 3:
				if (P1.length > 0) { P1.real_length--; }
				if (P2.length > 0) { P2.real_length--; }

				if (!blockIRQ) { irq(); }

				break;
			}
		}

		Pusle P1, P2;
		Triangle T1;
	};

	AudioData Audio;

	int tickCounter = 0;
	int frameCounter = 0;

	void audioCallBack(void *userdata, uint8_t *stream, int len)
	{
		AudioData* audio = (AudioData*)userdata;

		for (int i = 0; i < len; i += 2)
		{
			int OutCounter = 0;

			//Mode 0
			if (!audio->mode0) 
			{
				audio->P1.frequency = (double)(CPU) / (double)(16 * (audio->P1.timer + 1));
				audio->P2.frequency = (double)(CPU) / (double)(16 * (audio->P2.timer + 1));

				audio->T1.frequency = (double)(CPU) / (double)(16 * (audio->T1.timer + 1));

				double Out = 0;

				if (audio->enableP1) {
					//play wave based on duty for P1
					switch (audio->P1.duty)
					{
					case 0:
						if (sin((TAU * audio->P1.frequency) * audio->GTime) > 0 && sin((TAU * audio->P1.frequency) * audio->GTime) < 0.7071067812) { Out += 1.0; }
						else { Out += -1.0; }
						break;
					case 1:
						if (sin((TAU * audio->P1.frequency) * audio->GTime) > 0 && sin((TAU * audio->P1.frequency) * audio->GTime) < 1) { Out += 1.0; }
						else { Out += -1.0; }
						break;
					case 2:
						Out += (sin((TAU * audio->P1.frequency) * audio->GTime) > 0 ? 1.0 : -1.0);
						break;
					case 3:
						if (sin((TAU * audio->P1.frequency) * audio->GTime) > 0 && sin((TAU * audio->P1.frequency) * audio->GTime) < 1) { Out += 1.0; }
						else { Out += -1.0; }
						break;
					}
					OutCounter++;
				}
				if (audio->enableP2) {
					//play wave based on duty for P2
					switch (audio->P2.duty)
					{
					case 0:
						if (sin((TAU * audio->P2.frequency) * audio->GTime) > 0 && sin((TAU * audio->P2.frequency) * audio->GTime) < 0.7071067812) { Out += 1.0; }
						else { Out += -1.0; }
						break;
					case 1:
						if (sin((TAU * audio->P2.frequency) * audio->GTime) > 0 && sin((TAU * audio->P2.frequency) * audio->GTime) < 1) { Out += 1.0; }
						else { Out += -1.0; }
						break;
					case 2:
						Out += (sin((TAU * audio->P2.frequency) * audio->GTime) > 0 ? 1.0 : -1.0);
						break;
					case 3:
						if (sin((TAU * audio->P2.frequency) * audio->GTime) > 0 && sin((TAU * audio->P2.frequency) * audio->GTime) < 1) { Out += 1.0; }
						else { Out += -1.0; }
						break;
					}
					OutCounter++;
				}
				
				if (audio->enableTri) {
					//T1
					Out += asin(sin((TAU*audio->T1.frequency) * audio->GTime)) * (2.0 / PI);
					OutCounter++;
				}

				if (audio->enableNoise) 
				{
					Out += ((rand() % 1000) / 1000.0);
					OutCounter++;
				}

				Out = Out / OutCounter;
				Out *= 32000;

				// Out = 32000 * (sin((TAU * audio->frequency) * audio->GTime) > 0 ? 1.0 : -1.0);

				stream[i] = ((int)(Out) & 0xFF); 
				stream[i + 1] = ((int)(Out) >> 8);
			}
			//Mode 1
			else 
			{
				audio->P1.frequency = (double)(CPU) / (double)(16 * (audio->P1.timer + 1));
				audio->P2.frequency = (double)(CPU) / (double)(16 * (audio->P2.timer + 1));

				audio->T1.frequency = (double)(CPU) / (double)(16 * (audio->T1.timer + 1));

				double Out = 0;

				if (audio->enableP1) {
					//play wave based on duty for P1
					switch (audio->P1.duty)
					{
					case 0:
						if (sin((TAU * audio->P1.frequency) * audio->GTime) > 0 && sin((TAU * audio->P1.frequency) * audio->GTime) < 0.7071067812) { Out += 1.0; }
						else { Out += -1.0; }
						break;
					case 1:
						if (sin((TAU * audio->P1.frequency) * audio->GTime) > 0 && sin((TAU * audio->P1.frequency) * audio->GTime) < 1) { Out += 1.0; }
						else { Out += -1.0; }
						break;
					case 2:
						Out += (sin((TAU * audio->P1.frequency) * audio->GTime) > 0 ? 1.0 : -1.0);
						break;
					case 3:
						if (sin((TAU * audio->P1.frequency) * audio->GTime) > 0 && sin((TAU * audio->P1.frequency) * audio->GTime) < 1) { Out += 1.0; }
						else { Out += -1.0; }
						break;
					}
				}
				if (audio->enableP2) {
					//play wave based on duty for P2
					switch (audio->P2.duty)
					{
					case 0:
						if (sin((TAU * audio->P2.frequency) * audio->GTime) > 0 && sin((TAU * audio->P2.frequency) * audio->GTime) < 0.7071067812) { Out += 1.0; }
						else { Out += -1.0; }
						break;
					case 1:
						if (sin((TAU * audio->P2.frequency) * audio->GTime) > 0 && sin((TAU * audio->P2.frequency) * audio->GTime) < 1) { Out += 1.0; }
						else { Out += -1.0; }
						break;
					case 2:
						Out += (sin((TAU * audio->P2.frequency) * audio->GTime) > 0 ? 1.0 : -1.0);
						break;
					case 3:
						if (sin((TAU * audio->P2.frequency) * audio->GTime) > 0 && sin((TAU * audio->P2.frequency) * audio->GTime) < 1) { Out += 1.0; }
						else { Out += -1.0; }
						break;
					}
				}

				if (audio->enableTri) {
					//T1
					Out += asin(sin((TAU*audio->T1.frequency) * audio->GTime)) * (2.0 / PI);
				}

				Out = Out / 3;
				Out *= 32000;

				// Out = 32000 * (sin((TAU * audio->frequency) * audio->GTime) > 0 ? 1.0 : -1.0);

				stream[i] = ((int)(Out) & 0xFF);
				stream[i + 1] = ((int)(Out) >> 8);
			}

			//double Out = 32000 * sin((TAU*440) * audio->GTime); //MakeSound(audio->GTime, audio->Ocatave, audio->type);

			//stream[i] = ((int)(Out) & 0xFF);
			//stream[i + 1] = ((int)(Out) >> 8);

			audio->GTime += audio->TimeStep;
		}

		//cout << (int)audio->P1.real_length << endl;
	}

	void audioSetUp() 
	{
		if (SDL_Init(SDL_INIT_AUDIO) != 0) {
			std::cout << "unable to init SDL" << std::endl;
		}

		SDL_AudioSpec want, have;
		SDL_AudioDeviceID device;

		want.callback = audioCallBack;
		want.userdata = &Audio;
		want.freq = 48000;
		want.format = AUDIO_S16;
		want.channels = 1;
		want.samples = 1024;

		/*for (int e = 0; e < 5; e++)
		{
			for (int i = 0; i < 5; i++)
			{
				cout << SDL_GetAudioDeviceName(e, i) << endl;
			}
			cout << endl;
		}*/

		//need to firgurte out auto ditection, right now 0, 0 should be most computers
		device = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(3, 0), 0, &want, &have, 0);
		SDL_PauseAudioDevice(device, 0);
	}

	void tick() 
	{
		tickCounter++;

		if (tickCounter > 20545) 
		{
			tickCounter = 0;
			//tick the apu's system
			if (frameCounter > 3) { frameCounter = 0; }

			Audio.tick(frameCounter);

			frameCounter++;
		}
	}

	void UpdateReg(uint16_t loc, uint8_t data) 
	{
		//cout << "test Loc: " << hex << (int)loc << " " << (int)data;
		//system("pause");

		switch (loc) 
		{
		//Pusle wave 1
		case 0x4000:

			break;
		case 0x4001:
			break;
		case 0x4002:
			Audio.P1.timerL = 0;
			Audio.P1.timerL = data;
			break;
		case 0x4003:
			Audio.P1.timerH = 0;
			Audio.P1.timerH = data & 0x7;

			Audio.P1.length = (data >> 3);

			Audio.P1.real_length = APU_lookup_table[Audio.P1.length];
			break;
		//Pulse wave 2
		case 0x4004:

			break;
		case 0x4005:
			break;
		case 0x4006:
			Audio.P2.timerL = 0;
			Audio.P2.timerL = data;
			break;
		case 0x4007:
			Audio.P2.timerH = 0;
			Audio.P2.timerH = data & 0x7;

			Audio.P2.length = (data >> 3);

			Audio.P2.real_length = APU_lookup_table[Audio.P2.length];
			break;
		//triangle
		case 0x4008:

			break;
		case 0x4009:
			break;
		case 0x400A:
			Audio.T1.timerL = 0;
			Audio.T1.timerL = data;
			break;
		case 0x400B:
			Audio.T1.timerH = 0;
			Audio.T1.timerH = data & 0x7;

			break;
		//noise
		//dmc
		//Other
		//status
		case 0x4015:
			Audio.enableP1 = data & 0x1;
			Audio.enableP2 = ((data >> 1) & 0x1);
			Audio.enableTri = ((data >> 2) & 0x1);
			Audio.enableNoise = ((data >> 3) & 0x1);
			Audio.enableDMC = ((data >> 4) & 0x1);
			break;
		case 0x4017:
			Audio.mode0 = ((data >> 7) & 0x1);
			Audio.blockIRQ = ((data >> 7) & 0x1);
			break;
		}

		Audio.P1.timer = Audio.P1.timerL + (Audio.P1.timerH << 8);
		Audio.P2.timer = Audio.P2.timerL + (Audio.P2.timerH << 8);

		Audio.T1.timer = Audio.T1.timerL + (Audio.T1.timerH << 8);

		//length setup
		
		
	}
}
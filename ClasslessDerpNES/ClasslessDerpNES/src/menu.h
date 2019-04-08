#pragma once
#include <string>

struct Menu_options 
{
	char scalling = 2;
	std::string rom;
};

namespace menu {
	void guiRun();
	Menu_options* getSettings();
}
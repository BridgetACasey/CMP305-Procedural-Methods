// Main.cpp
#include "System.h"
#include "Application.h"

#include <ctime>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	Application* app = new Application();
	System* system;

	int t = std::time(0);

	srand(t);
	//srand(std::time(0));

	// Create the system object.
	system = new System(app, 1200, 675, true, false);

	// Initialize and run the system object.
	system->run();

	// Shutdown and release the system object.
	delete system;
	system = 0;

	return 0;
}
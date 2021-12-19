// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "LightShader.h"
#include "TerrainMesh.h"
#include "MarkovChain.h"

class Application : public BaseApplication
{
public:

	Application();
	~Application();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void gui();

private:
	LightShader* shader;
	TerrainMesh* m_Terrain;
	MarkovChain* markov;
	Light* light;

	std::string markovSentence;
};

#endif
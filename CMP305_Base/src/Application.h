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
	std::unique_ptr<LightShader> shader;
	std::unique_ptr<TerrainMesh> terrain;
	std::unique_ptr<Light> light;

	std::unique_ptr<MarkovChain> nameChain;
	std::unique_ptr<MarkovChain> descriptionChain;

	std::string markovName;
	std::string markovDescription;
};

#endif
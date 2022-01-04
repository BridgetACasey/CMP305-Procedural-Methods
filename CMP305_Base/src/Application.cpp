#include "Application.h"

Application::Application()
{
	terrain = nullptr;
	shader = nullptr;
	nameChain = nullptr;
	light = nullptr;
}

void Application::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Load textures
	textureMgr->loadTexture(L"grass", L"res/grass.png");
	textureMgr->loadTexture(L"white", L"res/DefaultDiffuse.png");
	textureMgr->loadTexture(L"wood", L"res/wood.png");
	textureMgr->loadTexture(L"snow", L"res/snow.png");
	textureMgr->loadTexture(L"sand", L"res/sand.jpg");

	// Create Mesh object and shader object
	terrain.reset(new TerrainMesh(renderer->getDevice(), renderer->getDeviceContext()));
	shader.reset(new LightShader(renderer->getDevice(), hwnd));
	
	// Initialise light
	light.reset(new Light());
	light->setDiffuseColour(1.0f, 0.9f, 0.9f, 1.0f);
	light->setDirection(0.5f, -1.0f, -0.5f);

	terrain->BuildHeightMap();

	nameChain.reset(new MarkovChain("name-corpus.txt", 3));
	descriptionChain.reset(new MarkovChain("description-corpus.txt", 4));

	markovName = nameChain->generateSentence("The", 36);
	markovDescription = descriptionChain->generateSentence("The ", 144);
}


Application::~Application()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();
}


bool Application::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}

	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool Application::render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	// Clear the scene. (default blue colour)
	renderer->beginScene(0.74f, 0.52f, 0.48f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	// Send geometry data, set shader parameters, render object with shader
	ID3D11ShaderResourceView* textures[] = { textureMgr->getTexture(L"sand"), textureMgr->getTexture(L"snow") };
	terrain->sendData(renderer->getDeviceContext());
	shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textures, light);
	shader->render(renderer->getDeviceContext(), terrain->getIndexCount());

	// Render GUI
	gui();

	// Swap the buffers
	renderer->endScene();

	if (startup)
	{
		terrain->renderSampleTerrain(timer->getTime());
		terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
		startup = false;
	}

	return true;
}

void Application::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("Name: ");
	ImGui::TextWrapped(markovName.c_str());
	ImGui::Spacing();
	ImGui::Text("Description: ");
	ImGui::TextWrapped(markovDescription.c_str());

	ImGui::Separator();
	ImGui::Spacing();

	static float cameraSpeed = 5.0f;

	ImGui::Text("General");
	ImGui::Spacing();
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Toggle Wireframe", &wireframeToggle);
	ImGui::DragFloat("Camera Speed", &cameraSpeed, 0.1f, 0.1f, 15.0f);
	ImGui::Spacing();
	if (ImGui::Button("Generate Sample Terrain"))
	{
		terrain->renderSampleTerrain(timer->getTime());
		terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}
	ImGui::Separator();
	ImGui::Spacing();

	camera->move(timer->getTime() * cameraSpeed);

	ImGui::Text("Lighting");
	ImGui::Spacing();

	static float diffuse[4] = { light->getDiffuseColour().x, light->getDiffuseColour().y, light->getDiffuseColour().z, light->getDiffuseColour().w };
	static float direction[3] = { light->getDirection().x, light->getDirection().y, light->getDirection().z };

	ImGui::DragFloat4("Diffuse", diffuse, 0.05f, 0.0f, 1.0f);
	ImGui::DragFloat3("Direction", direction, 0.1f, -1.0f, 1.0f);

	light->setDiffuseColour(diffuse[0], diffuse[1], diffuse[2], diffuse[3]);
	light->setDirection(direction[0], direction[1], direction[2]);

	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Markov Chain");

	ImGui::Spacing();

	if (ImGui::Button("Generate Name"))
	{
		markovName = nameChain->generateSentence("The", 36);
	}

	ImGui::SameLine();

	if (ImGui::Button("Generate Description"))
	{
		markovDescription = descriptionChain->generateSentence("The ", 144);
	}

	ImGui::Spacing();

	if (ImGui::Button("Generate All"))
	{
		markovName = nameChain->generateSentence("The", 36);
		markovDescription = descriptionChain->generateSentence("The ", 144);
	}

	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Terrain");
	ImGui::Spacing();

	static int terrainResolution = 128;
	static float amplitude = terrain->getAmplitude();
	static float frequency = terrain->getFrequency();

	ImGui::SliderInt("Resolution", &terrainResolution, 2, 1024);
	ImGui::DragFloat("Amplitude", &amplitude, 0.1f, 0.1f, 25.0f, "%.1f");
	ImGui::DragFloat("Frequency", &frequency, 0.001f, 0.001f, 0.999f, "%.3f");

	terrain->setAmplitude(amplitude);
	shader->setAmplitude(amplitude);
	terrain->setFrequency(frequency);

	if (ImGui::Button("Flatten"))
	{
		terrain->flatten();
		terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	ImGui::SameLine();

	if (ImGui::Button("Randomise"))
	{
		terrain->random();
		terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	ImGui::SameLine();

	if (ImGui::Button("Invert"))
	{
		terrain->invert();
		terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	static int smoothItrs = 1;
	if (ImGui::Button("Smooth"))
	{
		terrain->smooth(smoothItrs);
		terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}
	ImGui::SameLine();
	ImGui::DragInt("S Itr.", &smoothItrs, 1, 1, 10);

	static int faultItrs = 5;
	if (ImGui::Button("Fault "))
	{
		terrain->fault(faultItrs);
		terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}
	ImGui::SameLine();
	ImGui::DragInt("F Itr.", &faultItrs, 1, 1, 25);

	ImGui::Spacing();

	if (ImGui::Button("Regenerate Terrain"))
	{
		if (terrainResolution != terrain->GetResolution())
		{
			terrain->Resize(terrainResolution);
		}

		terrain->BuildHeightMap();
		terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Noise");
	ImGui::Spacing();

	static int octs = 8;
	ImGui::DragInt("Octaves", &octs, 1, 1, 10);

	static float amplInfl = 0.4f;
	ImGui::DragFloat("Ampl. Infl.", &amplInfl, 0.1f, 0.01f, 0.99f);

	static float freqInfl = 1.2f;
	ImGui::DragFloat("Freq. Infl.", &freqInfl, 0.1f, 0.01f, 10.0f);

	ImGui::Spacing();

	if (ImGui::Button("Original Perlin"))
	{
		terrain->perlinOriginal();
		terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}
	ImGui::SameLine();
	if (ImGui::Button("Improved Perlin"))
	{
		terrain->perlinImproved();
		terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	if (ImGui::Button("FBM"))
	{
		terrain->generateFBM(octs, amplInfl, freqInfl);
		terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}
	ImGui::SameLine();
	if (ImGui::Button("Ridged FBM"))
	{
		terrain->generateRigidFBM(octs, amplInfl, freqInfl);
		terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Wind Erosion");
	ImGui::Spacing();

	static bool weightedParticles = true;
	static int particleDensity = 250;
	static float particleVelocity[3] = { 1.0f, 0.0f, 1.0f };
	static float windVelocity[3] = { 3.0f, -3.0f, 3.0f };
	static float sediment = 0.01f;
	static float suspension = 0.002f;
	static float abrasion = 0.075f;
	static float roughness = 0.015f;
	static float settling = 0.05f;

	ImGui::Checkbox("Weighted Particles", &weightedParticles);
	ImGui::DragInt("P. Density", &particleDensity, 1, 1, 25000);
	ImGui::DragFloat3("Particle Vel.", particleVelocity, 0.1f, -10.0f, 10.0f);
	ImGui::DragFloat3("Wind Vel.", windVelocity, 0.1f, -10.0f, 10.0f);
	ImGui::DragFloat("Sediment", &sediment, 0.001f, 0.001f, 1.0f);
	ImGui::DragFloat("Suspension", &suspension, 0.001f, 0.001f, 1.0f);
	ImGui::DragFloat("Abrasion", &abrasion, 0.001f, 0.001f, 1.0f);
	ImGui::DragFloat("Roughness", &roughness, 0.001f, 0.001f, 1.0f);
	ImGui::DragFloat("Settling", &settling, 0.001f, 0.001f, 1.0f);

	ImGui::Spacing();

	if (ImGui::Button("Apply Wind Erosion"))
	{
		terrain->windErosion(timer->getTime(), particleDensity, particleVelocity,
			windVelocity, sediment, suspension, abrasion, roughness, settling, weightedParticles);
		terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	ImGui::Separator();

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
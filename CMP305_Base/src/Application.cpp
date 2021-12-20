#include "Application.h"

Application::Application()
{
	m_Terrain = nullptr;
	shader = nullptr;
	markov = nullptr;
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
	m_Terrain = new TerrainMesh(renderer->getDevice(), renderer->getDeviceContext());
	shader = new LightShader(renderer->getDevice(), hwnd);
	
	// Initialise light
	light = new Light();
	light->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	light->setDirection(0.5f, -1.0f, -0.5f);

	m_Terrain->BuildHeightMap();
	m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());

	markov = new MarkovChain("corpus.txt", 5);
	markovSentence = markov->generateSentence("women", 1000);
}


Application::~Application()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.
	if (m_Terrain)
	{
		delete m_Terrain;
		m_Terrain = 0;
	}

	if (shader)
	{
		delete shader;
		shader = 0;
	}
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
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	// Send geometry data, set shader parameters, render object with shader
	ID3D11ShaderResourceView* textures[] = { textureMgr->getTexture(L"sand"), textureMgr->getTexture(L"snow") };
	m_Terrain->sendData(renderer->getDeviceContext());
	shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textures, light);
	shader->render(renderer->getDeviceContext(), m_Terrain->getIndexCount());

	// Render GUI
	gui();

	// Swap the buffers
	renderer->endScene();

	return true;
}

void Application::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	//ImGui::TextWrapped(markovSentence.c_str());
	static float cameraSpeed = 5.0f;

	ImGui::Text("General");
	ImGui::Spacing();
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Toggle Wireframe", &wireframeToggle);
	ImGui::DragFloat("Camera Speed", &cameraSpeed, 0.1f, 0.1f, 15.0f);
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

	ImGui::Text("Terrain");
	ImGui::Spacing();

	static int terrainResolution = 128;
	static float amplitude = m_Terrain->getAmplitude();
	static float frequency = m_Terrain->getFrequency();

	if (ImGui::Button("Regenerate Terrain"))
	{
		if (terrainResolution != m_Terrain->GetResolution())
		{
			m_Terrain->Resize(terrainResolution);
		}

		m_Terrain->BuildHeightMap();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	ImGui::SliderInt("Resolution", &terrainResolution, 2, 1024);
	ImGui::DragFloat("Amplitude", &amplitude, 0.1f, 0.1f, 25.0f, "%.1f");
	ImGui::DragFloat("Frequency", &frequency, 0.01f, 0.01f, 0.99f, "%.2f");

	m_Terrain->setAmplitude(amplitude);
	shader->setAmplitude(amplitude);
	m_Terrain->setFrequency(frequency);

	ImGui::Spacing();

	if (ImGui::Button("Flatten"))
	{
		m_Terrain->flatten();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	ImGui::SameLine();

	if (ImGui::Button("Randomise"))
	{
		m_Terrain->random();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	ImGui::SameLine();

	if (ImGui::Button("Invert"))
	{
		m_Terrain->invert();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	ImGui::SameLine();

	if (ImGui::Button("Smooth"))
	{
		m_Terrain->smooth();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	ImGui::SameLine();

	if (ImGui::Button("Fault"))
	{
		m_Terrain->fault();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	if (ImGui::Button("Particle Deposition"))
	{
		m_Terrain->particleDeposition();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());

	}

	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Wind Erosion");
	ImGui::Spacing();

	static int particleScale = 50000;
	static int windIterations = 500;

	ImGui::DragInt("Particle Scale", &particleScale, 500, 1000, 100000);
	ImGui::DragInt("Wind Iterations", &windIterations, 1, 1, 25000);

	if (ImGui::Button("Wind Erosion"))
	{
		m_Terrain->windErosion(timer->getTime(), windIterations, particleScale);
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Noise");
	ImGui::Spacing();

	if (ImGui::Button("Original Perlin"))
	{
		m_Terrain->originalPerlin();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	if (ImGui::Button("1D Perlin"))
	{
		m_Terrain->perlin1D();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	ImGui::SameLine();

	if (ImGui::Button("2D Perlin"))
	{
		m_Terrain->perlin2D();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	ImGui::Spacing();

	static int octs = 8;
	ImGui::DragInt("Octaves", &octs, 1, 1, 10);

	static float amplInfl = 0.4f;
	ImGui::DragFloat("Ampl. Infl.", &amplInfl, 0.1f, 0.01f, 0.99f);

	static float freqInfl = 2.0f;
	ImGui::DragFloat("Freq. Infl.", &freqInfl, 0.1f, 1.0f, 10.0f);

	if (ImGui::Button("Fractional Brownian Motion"))
	{
		m_Terrain->generateFBM(octs, amplInfl, freqInfl);
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	if (ImGui::Button("Rigid FBM"))
	{
		m_Terrain->generateRigidFBM(octs, amplInfl, freqInfl);
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	ImGui::Separator();

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
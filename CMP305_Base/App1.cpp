#include "App1.h"

App1::App1()
{
	m_Terrain = nullptr;
	shader = nullptr;
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Load textures
	textureMgr->loadTexture(L"grass", L"res/grass.png");
	textureMgr->loadTexture(L"white", L"res/DefaultDiffuse.png");
	textureMgr->loadTexture(L"wood", L"res/wood.png");

	// Create Mesh object and shader object
	m_Terrain = new TerrainMesh(renderer->getDevice(), renderer->getDeviceContext());
	shader = new LightShader(renderer->getDevice(), hwnd);
	
	// Initialise light
	light = new Light();
	light->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	light->setDirection(1.0f, 0.0f, 0.0f);

	m_Terrain->BuildHeightMap();
	m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
}


App1::~App1()
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


bool App1::frame()
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

bool App1::render()
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
	m_Terrain->sendData(renderer->getDeviceContext());
	ID3D11ShaderResourceView* textures[] = { textureMgr->getTexture(L"grass"), textureMgr->getTexture(L"wood") };
	shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textures, light);
	shader->render(renderer->getDeviceContext(), m_Terrain->getIndexCount());

	// Render GUI
	gui();

	// Swap the buffers
	renderer->endScene();

	return true;
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	ImGui::SliderInt("Terrain Resolution", &terrainResolution, 2, 1024);
	
	static float frequency = m_Terrain->getFrequency();
	static float amplitude = m_Terrain->getAmplitude();

	ImGui::DragFloat("Amplitude", &amplitude, 0.25f, 1.0f, 100.0f);
	m_Terrain->setAmplitude(amplitude);
	shader->setAmplitude(amplitude);

	ImGui::DragFloat("Frequency", &frequency, 0.025f, 0.001f, 0.99f);
	m_Terrain->setFrequency(frequency);

	if (ImGui::Button("Reset"))
	{
		m_Terrain->reset();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	if (ImGui::Button("Invert"))
	{
		m_Terrain->invert();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	if (ImGui::Button("Smooth"))
	{
		m_Terrain->smooth();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	if (ImGui::Button("Randomise"))
	{
		m_Terrain->random();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	if (ImGui::Button("Fault"))
	{
		m_Terrain->fault();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	if (ImGui::Button("Particle"))
	{
		m_Terrain->particleDeposition(0.25f);
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	if (ImGui::Button("Perlin"))
	{
		m_Terrain->perlin2D();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	static int octs = 8;
	ImGui::DragInt("Octaves", &octs, 1, 1, 10);

	static float amplInfl = 0.4f;
	ImGui::DragFloat("Ampl. Infl.", &amplInfl, 0.1f, 0.01f, 0.99f);

	static float freqInfl = 2.0f;
	ImGui::DragFloat("Freq. Infl.", &freqInfl, 0.1f, 1.0f, 10.0f);

	if (ImGui::Button("FBM"))
	{
		m_Terrain->generateFBM(octs, freqInfl, amplInfl, 0.0f, 0.0f);
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	if (ImGui::Button("Rigid FBM"))
	{
		m_Terrain->generateRigidFBM(octs, freqInfl, amplInfl, 0.0f, 0.0f);
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	if (ImGui::Button("Regenerate Terrain"))
	{
		if (terrainResolution != m_Terrain->GetResolution())
		{
			m_Terrain->Resize(terrainResolution);
		}

		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
#pragma once
#include "PlaneMesh.h"
#include "PerlinNoise.h"
#include "WindErosion.h"

class TerrainMesh : public PlaneMesh
{
public:
	TerrainMesh( ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 128 );
	~TerrainMesh();

	void BuildHeightMap();

	void Resize( int newResolution );
	void Regenerate( ID3D11Device* device, ID3D11DeviceContext* deviceContext );

	void flatten();
	void invert();

	void smooth();
	void random();
	void fault();

	void particleDeposition();
	void windErosion(float deltaTime, int itr, float* pVel, float* wVel, float sed, float sus, float abr, float rgh, float set);

	void originalPerlin();
	void perlin1D();
	void perlin2D();

	void generateFBM(int octaves, float ampl, float freq);
	void generateRigidFBM(int octaves, float freq, float ampl);

	const inline int GetResolution() { return resolution; }

	void setAmplitude(float ampl) { amplitude = ampl; }
	void setFrequency(float freq) { frequency = freq; }
	float getFrequency() const { return frequency; }
	int getAmplitude() const { return amplitude; }

private:
	void CreateBuffers( ID3D11Device* device, VertexType* vertices, unsigned long* indices );

	const float m_UVscale = 10.0f;			//Tile the UV map 10 times across the plane
	const float terrainSize = 100.0f;		//What is the width and height of our terrain
	float* heightMap;
	float* sedimentMap;

	float amplitude;
	float frequency;

	PerlinNoise perlin;
};

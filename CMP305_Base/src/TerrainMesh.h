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

	void renderSampleTerrain(float dt);

	void flatten();
	void invert();
	void random();

	void smooth(int itr);
	void fault(int itr);

	void perlinOriginal();
	void perlinImproved();

	void generateFBM(int octaves, float ampl, float freq);
	void generateRidgedFBM(int octaves, float freq, float ampl);

	void windErosion(float dt, int itr, float* pVel, float* wVel, float sed, float sus, float abr, float rgh, float set, bool weigh);

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

	PerlinNoise noise;
};

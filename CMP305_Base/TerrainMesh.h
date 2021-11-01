#pragma once
#include "PlaneMesh.h"
#include "PerlinNoise.h"

class TerrainMesh : public PlaneMesh
{
public:
	TerrainMesh( ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 128 );
	~TerrainMesh();

	void Resize( int newResolution );
	void Regenerate( ID3D11Device* device, ID3D11DeviceContext* deviceContext );

	const inline int GetResolution(){ return resolution; }

	void setAmplitude(float ampl) { amplitude = ampl; }
	void setFrequency(float freq) { frequency = freq; }
	float getFrequency() const { return frequency; }
	int getAmplitude() const { return amplitude; }

	void BuildHeightMap();

	void reset();
	void invert();

	void smooth();
	void random();
	void fault();
	void particleDeposition(float particleHeight);

	void perlin2D();
	void generateFBM(int octaves, float freq, float ampl, float offsetX, float offsetZ);
	void generateRigidFBM(int octaves, float freq, float ampl, float offsetX, float offsetZ);

private:
	void CreateBuffers( ID3D11Device* device, VertexType* vertices, unsigned long* indices );

	const float m_UVscale = 10.0f;			//Tile the UV map 10 times across the plane
	const float terrainSize = 100.0f;		//What is the width and height of our terrain
	float* heightMap;

	float amplitude;
	float frequency;

	PerlinNoise* perlin;
};

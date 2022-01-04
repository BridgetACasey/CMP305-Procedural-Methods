#pragma once

#include "DXF.h"

struct WindParticle
{
	XMFLOAT2 position = { 0.0f, 0.0f };
	XMFLOAT3 velocity = { 1.0f, 0.0f, 1.0f };
};

class WindErosion
{
public:
	WindErosion();
	WindErosion(float vx, float vy, float vz);
	~WindErosion();

	void setWindAttributes(float sed, float sus, float abr, float rgh, float set, bool weigh);
	void fly(float deltaTime, float amplitude, float* heightMap, float* sedimentMap, WindParticle& particle, int resolution);

private:
	void cascade(float deltaTime, int i, float* heightMap, float* sedimentMap, WindParticle& particle, int resolution);
	XMFLOAT2 calculateParticleWeight(float deltaTime, WindParticle& particle, XMFLOAT2 posIncrement, float* heightMap, int index, int resolution);

	XMFLOAT3 windVelocity = { 3.0f, -3.0f, 3.0f };

	bool weightedParticles = true;
	float height = 0.0f;
	float sedimentRate = 0.01f;
	float suspension = 0.002f;
	float abrasion = 0.01f;
	float roughness = 0.005f;
	float settling = 0.05f;
};
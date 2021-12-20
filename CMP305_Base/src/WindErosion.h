#pragma once

#include "DXF.h"

struct Particle
{
	XMFLOAT2 position = { 0.0f, 0.0f };
	XMFLOAT3 velocity = { 1.0f, 0.0f, 1.0f };
};

class WindErosion
{
public:
	WindErosion();
	~WindErosion();

	void fly(float deltaTime, float amplitude, float* heightMap, float* sedimentMap, Particle& particle, int resolution, float scale);

private:
	void cascade(float deltaTime, int i, float* heightMap, float* sedimentMap, Particle& particle, int resolution);

	XMFLOAT3 windVelocity = { 3.0f, -3.0f, 3.0f };

	float height = 0.0f;
	float sedimentRate = 0.01f;
	float suspension = 0.002f;
	float abrasion = 0.01f;
	float roughness = 0.005f;
	float settling = 0.05f;
};
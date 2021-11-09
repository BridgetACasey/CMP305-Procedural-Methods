#pragma once

#include <vector>

class PerlinNoise
{
public:
	PerlinNoise();
	~PerlinNoise();

	float generateNoise1D(float x);
	float generateNoise2D(float x, float y);
	float generateNoise3D(float x, float y, float z);

private:
	std::vector<int> permutationTable;

	std::vector<float> gradientTable1D;
	std::vector<std::pair<float, float>> gradientTable2D;

	void setupPermutationTable();
	void setupGradientTables();

	float fade(float t);
};
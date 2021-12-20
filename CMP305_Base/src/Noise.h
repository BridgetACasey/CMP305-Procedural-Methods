#pragma once

#include <vector>

class Noise
{
public:
	Noise();
	~Noise();

	float generatePerlin1D(float point);
	float generatePerlin2D(float x, float y);
	float generatePerlin3D(float x, float y, float z);
	float generateImprovedPerlin(float x, float y);

private:
	std::vector<int> permutationTable;

	std::vector<float> gradientTable1D;
	std::vector<std::pair<float, float>> gradientTable2D;

	void setupPermutationTable();
	void setupGradientTables();

	float fade(float t);
};
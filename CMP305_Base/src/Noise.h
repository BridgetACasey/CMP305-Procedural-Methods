#pragma once

#include "DXF.h"
#include <vector>

class Noise
{
public:
	Noise();
	~Noise();

	float generatePerlin1D(float point);
	float generatePerlin2D(float x, float y);
	float generateImprovedPerlin(float x, float y);

private:
	std::vector<int> permutationTable;

	std::vector<float> gradientTable1D;
	std::vector<std::pair<float, float>> gradientTable2D;

	float generateRandomGradient(int hash, float x, float y);
	XMFLOAT2 generateGrad(int hash, float x, float y);

	void setupPermutationTable();
	void setupGradientTables();
};
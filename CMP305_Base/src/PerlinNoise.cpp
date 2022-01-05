#include "PerlinNoise.h"

#include <cmath>
#include <algorithm>

#include "MathsUtils.h"

PerlinNoise::PerlinNoise()
{
	setupPermutationTable();
	setupGradientTables();
}

PerlinNoise::~PerlinNoise()
{

}

float PerlinNoise::generatePerlin1D(float point)
{
	point = MathsUtils::clamp(point, 0.0f, 511.0f);

	int min = (int)point;
	int max = (int)point + 1;

	float remainders[2] = { point - (float)min, point - (float)max };

	float easing = MathsUtils::fadeOriginal(remainders[0]);

	int i = permutationTable.at(min);
	int j = permutationTable.at(max);

	int gradIndex[2] =
	{
		permutationTable.at(MathsUtils::clamp(i + min, 0, 511)),
		permutationTable.at(MathsUtils::clamp(j + max, 0, 511))
	};

	float u = remainders[0] * gradientTable1D.at(gradIndex[0]);
	float v = remainders[1] * gradientTable1D.at(gradIndex[1]);

	return MathsUtils::interpolate(u, v, easing);
}

float PerlinNoise::generatePerlin2D(float x, float y)
{
	//Take point on height map and scale by frequency
	//Pass into perlin function
	//x & y = the new point we have selected in between grid points (i.e. a decimal value)
	//Find surrounding grid points of x,y (i.e. whole numbers)
	//Assign pseudorandom gradient vectors to each point
	//Interpolate between surrounding grid points based on new point
	//Final scalar output is the value for the height map

	int xMin = (int)x;
	int yMin = (int)y;
	int xMax = (int)x + 1;
	int yMax = (int)y + 1;

	int i = permutationTable.at(MathsUtils::clamp(xMin, 0, 511));
	int j = permutationTable.at(MathsUtils::clamp(xMax, 0, 511));

	int gradIndex[4] =
	{
		permutationTable.at(MathsUtils::clamp(i + yMin, 0, 511)),
		permutationTable.at(MathsUtils::clamp(j + yMin, 0, 511)),
		permutationTable.at(MathsUtils::clamp(i + yMax, 0, 511)),
		permutationTable.at(MathsUtils::clamp(j + yMax, 0, 511))
	};

	float remainders[4] = { x - xMin, y - yMin, x - xMax, y - yMax };

	float easingX = MathsUtils::fadeOriginal(remainders[0]);
	float easingY = MathsUtils::fadeOriginal(remainders[1]);

	float gradX, gradY, u, v, a, b;

	gradX = gradientTable2D.at(gradIndex[0]).first;
	gradY = gradientTable2D.at(gradIndex[0]).second;
	u = MathsUtils::scalarProduct(remainders[0], remainders[1], gradX, gradY);

	gradX = gradientTable2D.at(gradIndex[1]).first;
	gradY = gradientTable2D.at(gradIndex[1]).second;
	v = MathsUtils::scalarProduct(remainders[2], remainders[1], gradX, gradY);

	a = MathsUtils::interpolate(u, v, easingX);

	gradX = gradientTable2D.at(gradIndex[2]).first;
	gradY = gradientTable2D.at(gradIndex[2]).second;
	u = MathsUtils::scalarProduct(remainders[0], remainders[3], gradX, gradY);

	gradX = gradientTable2D.at(gradIndex[3]).first;
	gradY = gradientTable2D.at(gradIndex[3]).second;
	v = MathsUtils::scalarProduct(remainders[2], remainders[3], gradX, gradY);

	b = MathsUtils::interpolate(u, v, easingX);

	return MathsUtils::interpolate(a, b, easingY);
}

float PerlinNoise::generateImprovedPerlin(float x, float y)
{
	int xMin = (int)x;
	int yMin = (int)y;
	int xMax = (int)x + 1;
	int yMax = (int)y + 1;

	float remainders[4] = { x - xMin, y - yMin, x - xMax, y - yMax };

	float easingX = MathsUtils::fadeImproved(remainders[0]);
	float easingY = MathsUtils::fadeImproved(remainders[1]);

	int i = permutationTable.at(MathsUtils::clamp(xMin, 0, 511));
	int j = permutationTable.at(MathsUtils::clamp(xMax, 0, 511));
	int k = permutationTable.at(MathsUtils::clamp(yMin, 0, 511));
	int l = permutationTable.at(MathsUtils::clamp(yMax, 0, 511));

	XMFLOAT2 gradient;
	float u, v, a, b;

	gradient = generateGrad(permutationTable.at(MathsUtils::clamp(i + yMin, 0, 511)), remainders[0], remainders[1]);
	u = MathsUtils::scalarProduct(remainders[0], remainders[1], gradient.x, gradient.y);
	gradient = generateGrad(permutationTable.at(MathsUtils::clamp(j + yMin, 0, 511)), remainders[2], remainders[1]);
	v = MathsUtils::scalarProduct(remainders[2], remainders[1], gradient.x, gradient.y);
	a = MathsUtils::interpolate(u, v, easingX);

	gradient = generateGrad(permutationTable.at(MathsUtils::clamp(i + yMax, 0, 511)), remainders[0], remainders[3]);
	u = MathsUtils::scalarProduct(remainders[0], remainders[3], gradient.x, gradient.y);
	gradient = generateGrad(permutationTable.at(MathsUtils::clamp(j + yMax, 0, 511)), remainders[2], remainders[3]);
	v = MathsUtils::scalarProduct(remainders[2], remainders[3], gradient.x, gradient.y);
	b = MathsUtils::interpolate(u, v, easingX);

	return MathsUtils::interpolate(a, b, easingY);
}

XMFLOAT2 PerlinNoise::generateGrad(int hash, float x, float y)
{
	int h = hash & 15;

	float u = h < 8 ? x : y;
	float v = h < 4 ? y : h == 12 || h == 14 ? x : 0;

	u = (h & 2) == 0 ? u : -u;
	v = (h & 2) == 0 ? v : -v;

	return XMFLOAT2(u, v);
}

void PerlinNoise::setupPermutationTable()
{
	//Assigning values uniformly to permutation table
	for (int i = 0; i < 512; i++)
	{
		permutationTable.push_back(i);
	}

	std::random_shuffle(permutationTable.begin(), permutationTable.end());
}

void PerlinNoise::setupGradientTables()
{
	for (int j = 0; j < 512; j++)
	{
		float gradX = (float)(rand() % 512);
		float gradY = (float)(rand() % 512);

		gradX = ((gradX - 256.0f) / 256.0f);
		gradY = ((gradY - 256.0f) / 256.0f);

		MathsUtils::normalise2D(gradX, gradY);

		gradientTable1D.push_back(gradX);
		gradientTable2D.push_back(std::pair<float, float>(gradX, gradY));
	}
}
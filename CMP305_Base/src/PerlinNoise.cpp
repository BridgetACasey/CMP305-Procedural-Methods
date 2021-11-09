#include "PerlinNoise.h"

#include <cmath>
#include <algorithm>

#include "MathsUtils.h"

struct PerlinCell2D
{
	int xMin = 0;
	int xMax = 0;
	int yMin = 0;
	int yMax = 0;
};

PerlinNoise::PerlinNoise()
{
	setupPermutationTable();
	setupGradientTables();
}

PerlinNoise::~PerlinNoise()
{

}

//TO FIX
float PerlinNoise::generateNoise1D(float x)
{
	return 0.0f;
}

float PerlinNoise::generateNoise2D(float x, float y)
{
	//Take point on height map and scale by frequency
	//Pass into perlin function
	//x & y = the new point we have selected in between grid points (i.e. a decimal value)
	//Find surrounding grid points of x,y (i.e. whole numbers)
	//Assign pseudorandom gradient vectors to each point
	//Interpolate between surrounding grid points based on new point
	//Final scalar output is the value for the height map

	PerlinCell2D current;

	int gradIndex[4];

	current.xMin = (int)x;
	current.yMin = (int)y;
	current.xMax = (int)x + 1;
	current.yMax = (int)y + 1;

	float remainders[4] = { x - current.xMin, y - current.yMin, current.xMax - x, current.yMax - y };

	int i = permutationTable.at(current.xMin);
	int j = permutationTable.at(current.xMax);

	gradIndex[0] = permutationTable.at(MathsUtils::clamp(i + current.yMin, 0, 255));
	gradIndex[1] = permutationTable.at(MathsUtils::clamp(j + current.yMin, 0, 255));
	gradIndex[2] = permutationTable.at(MathsUtils::clamp(i + current.yMax, 0, 255));
	gradIndex[3] = permutationTable.at(MathsUtils::clamp(j + current.yMax, 0, 255));

	float easingX = fade(remainders[0]);
	float easingY = fade(remainders[1]);

	float gradX, gradY, u, v, a, b;

	gradX = gradientTable2D.at(gradIndex[0]).first;
	gradY = gradientTable2D.at(gradIndex[0]).second;
	u = MathsUtils::scalarProduct(remainders[0], remainders[1], gradX, gradY);

	gradX = gradientTable2D.at(gradIndex[2]).first;
	gradY = gradientTable2D.at(gradIndex[2]).second;
	v = MathsUtils::scalarProduct(remainders[2], remainders[1], gradX, gradY);

	a = MathsUtils::interpolate(u, v, easingX);

	gradX = gradientTable2D.at(gradIndex[1]).first;
	gradY = gradientTable2D.at(gradIndex[1]).second;
	u = MathsUtils::scalarProduct(remainders[0], remainders[3], gradX, gradY);

	gradX = gradientTable2D.at(gradIndex[3]).first;
	gradY = gradientTable2D.at(gradIndex[3]).second;
	v = MathsUtils::scalarProduct(remainders[2], remainders[3], gradX, gradY);

	b = MathsUtils::interpolate(u, v, easingY);

	return MathsUtils::interpolate(a, b, easingY);
}

//To add at a later date
float PerlinNoise::generateNoise3D(float x, float y, float z)
{
	return 0.0f;
}

void PerlinNoise::setupPermutationTable()
{
	//Assigning values from 0-255 uniformly to permutation table
	for (int i = 0; i < 256; i++)
	{
		permutationTable.push_back(i);
	}

	std::random_shuffle(permutationTable.begin(), permutationTable.end());
}

void PerlinNoise::setupGradientTables()
{
	for (int i = 0; i < 256; i++)
	{
		int gradX = rand() % 255;
		int gradY = rand() % 255;

		gradientTable1D.push_back((float)gradX / 255);
		gradientTable2D.push_back(std::pair<float, float>((float)gradX / 255, (float)gradY / 255));
	}

	for (int j = 0; j < ((512 + 2) * 2); j++)
	{
		int gradX = rand() % 255;
		int gradY = rand() % 255;

		gradientTable2D.push_back(std::pair<float, float>((float)gradX / 255, (float)gradY / 255));
	}
}

float PerlinNoise::fade(float t)
{
	return (t * t * (3.0f - 2.0f * t));
}
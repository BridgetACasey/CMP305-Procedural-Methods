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

//TO FIX
float PerlinNoise::generateNoise1D(float point)
{
	point = MathsUtils::clamp(point, 0.0f, 510.0f);

	int min = (int)point;
	int max = (int)point + 1;

	float remainders[2] = { point - (float)min, point - (float)max };

	float easing = fade(remainders[0]);

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

float PerlinNoise::generateNoise2D(float x, float y)
{
	//Take point on height map and scale by frequency
	//Pass into perlin function
	//x & y = the new point we have selected in between grid points (i.e. a decimal value)
	//Find surrounding grid points of x,y (i.e. whole numbers)
	//Assign pseudorandom gradient vectors to each point
	//Interpolate between surrounding grid points based on new point
	//Final scalar output is the value for the height map

	x = MathsUtils::clamp(x, 0.0f, 510.0f);
	y = MathsUtils::clamp(y, 0.0f, 510.0f);

	int xMin = (int)x;
	int yMin = (int)y;
	int xMax = (int)x + 1;
	int yMax = (int)y + 1;

	int i = permutationTable.at(xMin);
	int j = permutationTable.at(xMax);

	int gradIndex[4] =
	{
		permutationTable.at(MathsUtils::clamp(i + yMin, 0, 511)),
		permutationTable.at(MathsUtils::clamp(j + yMin, 0, 511)),
		permutationTable.at(MathsUtils::clamp(i + yMax, 0, 511)),
		permutationTable.at(MathsUtils::clamp(j + yMax, 0, 511))
	};

	float remainders[4] = { x - xMin, y - yMin, x - xMax, y - yMax };

	float easingX = fade(remainders[0]);
	float easingY = fade(remainders[1]);

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

//To add at a later date
float PerlinNoise::generateNoise3D(float x, float y, float z)
{
	return 0.0f;
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

float PerlinNoise::fade(float t)
{
	return (t * t * (3.0f - 2.0f * t));
}
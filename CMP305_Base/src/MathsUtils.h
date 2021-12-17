#pragma once

#include <cstdlib>

class MathsUtils
{
public:
	static const inline int clamp(int value, int lower, int upper)
	{
		if (value < lower)
		{
			return lower;
		}

		if (value > upper)
		{
			return upper;
		}

		return value;
	}
	
	static const inline float clamp(float value, float lower, float upper)
	{
		if (value < lower)
		{
			return lower;
		}

		if (value > upper)
		{
			return upper;
		}

		return value;
	}

	static const inline float interpolate(float first, float second, float weight)
	{
		return first + weight * (second - first);
	}

	static const inline float scalarProduct(float x1, float y1, float x2, float y2)
	{
		return (x1 * x2) + (y1 * y2);
	}

	static const inline void normalise2D(float& x, float& y)
	{
		float root = sqrt((x * x) + (y * y));

		x /= root;
		y /= root;
	}

	static const inline void normalise3D(float x, float y, float z)
	{
		float root = sqrt((x * x) + (y * y) + (z * z));

		x /= root;
		y /= root;
	}
};
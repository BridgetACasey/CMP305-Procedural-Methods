#pragma once

#include <cstdlib>
#include "DXF.h"

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

	static const inline float magnitude1(float x)
	{
		float result = x * x;

		return sqrt(result);
	}

	static const inline float magnitude2(float x, float y)
	{
		float result = (x * x) + (y * y);

		return sqrt(result);
	}

	static const inline float magnitude3(float x, float y, float z)
	{
		float result = (x * x) + (y * y) + (z * z);

		return sqrt(result);
	}

	static const inline float scalarProduct(float x1, float y1, float x2, float y2)
	{
		return (x1 * x2) + (y1 * y2);
	}

	static const inline XMFLOAT3 crossProduct(XMFLOAT3 a, XMFLOAT3 b)
	{
		XMFLOAT3 result;

		result.x = (a.y * b.z) - (a.z * b.y);
		result.y = (a.z * b.x) - (a.x * b.z);
		result.z = (a.x * b.y) - (a.y * b.x);

		return result;
	}

	static const inline void normalise2D(float& x, float& y)
	{
		float root = sqrt((x * x) + (y * y));

		x /= root;
		y /= root;
	}

	static const inline void normalise3D(float& x, float& y, float& z)
	{
		float root = sqrt((x * x) + (y * y) + (z * z));

		x /= root;
		y /= root;
	}

	static const inline void normalise3D(XMFLOAT3& v)
	{
		//float root = sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
		//
		//v.x /= root;
		//v.y /= root;

		float length = magnitude3(v.x, v.y, v.z);

		v.x /= length;
		v.y /= length;
		v.z /= length;
	}

	static const inline float fadeOriginal(float t)
	{
		return (t * t * (3.0f - 2.0f * t));
	}

	static const inline float fadeImproved(float t)
	{
		return  t * t * t * (t * (t * 6 - 15) + 10);
	}

	static const inline XMFLOAT2 modulus(XMFLOAT2& a, XMFLOAT2 b)
	{
		XMFLOAT2 result;

		result.x = a.x - b.x * (floor(a.x / b.x));
		result.y = a.y - b.y * (floor(a.y / b.y));

		return result;
	}

	static const inline float floor(float a)
	{
		int b = (int)a;

		return (float)b;
	}
};
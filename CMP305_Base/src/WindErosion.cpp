#include "WindErosion.h"

#include "MathsUtils.h"

XMFLOAT2 operator +(const XMFLOAT2& lhs, const XMFLOAT2& rhs)
{
	XMFLOAT2 result;

	result.x = lhs.x + rhs.x;
	result.y = lhs.y + rhs.y;

	return result;
}

XMFLOAT3 operator +(const XMFLOAT3& lhs, const XMFLOAT3& rhs)
{
	XMFLOAT3 result;

	result.x = lhs.x + rhs.x;
	result.y = lhs.y + rhs.y;
	result.z = lhs.z + rhs.z;

	return result;
}

XMINT2 operator +(const XMINT2& lhs, const XMINT2& rhs)
{
	XMINT2 result;

	result.x = lhs.x + rhs.x;
	result.y = lhs.y + rhs.y;

	return result;
}

WindErosion::WindErosion()
{
}

WindErosion::WindErosion(float vx, float vy, float vz)
{
	windVelocity.x = vx;
	windVelocity.y = vy;
	windVelocity.z = vz;
}

WindErosion::~WindErosion()
{
}

void WindErosion::setWindAttributes(float sed, float sus, float abr, float rgh, float set, bool weigh)
{
	weightedParticles = weigh;
	sedimentRate = sed;
	suspension = sus;
	abrasion = abr;
	roughness = rgh;
	settling = set;
}

void WindErosion::fly(float dt, float amplitude, float* heightMap, float* sedimentMap, WindParticle& particle, int resolution)
{
	float acceleration = 0.1f;
	int index = ((int)particle.position.y * resolution) + (int)particle.position.x;

	while (true)
	{
		if (height < (heightMap[index] + sedimentMap[index]))
		{
			//Particles underneath the heightmap are moved upwards
			height = (heightMap[index] + sedimentMap[index]);
		}

		else if (height > heightMap[index] + sedimentMap[index])
		{
			//Applying gravity to flying particles
			windVelocity.y -= (dt * acceleration);
		}

		else
		{
			//Calculating the deflection normal with respect to a wind direction
			XMFLOAT3 normal = calculateDeflectionNormal(particle, index, resolution, heightMap, sedimentMap, amplitude);

			if (weightedParticles)
			{
				XMFLOAT3 cross = MathsUtils::crossProduct(MathsUtils::crossProduct(windVelocity, normal), normal);

				MathsUtils::normalise3D(cross);

				windVelocity.x = dt * cross.x;
				windVelocity.y = dt * cross.y;
				windVelocity.z = dt * cross.z;
			}
		}

		//Accelerate wind
		windVelocity.x += (acceleration * dt * (particle.velocity.x - windVelocity.x));
		windVelocity.y += (acceleration * dt * (particle.velocity.y - windVelocity.y));
		windVelocity.z += (acceleration * dt * (particle.velocity.z - windVelocity.z));

		particle.position.x += dt * windVelocity.x;
		particle.position.y += dt * windVelocity.z;

		height += (dt * windVelocity.y);

		//The next index position of the particle
		int nextIndex = ((int)particle.position.y * resolution) + (int)particle.position.x;

		//Check that the particle hasn't moved to an out of bounds position
		if (nextIndex < 0 || nextIndex > (resolution * resolution))
		{
			break;
		}

		//Made contact with the surface of the terrain
		if (height <= heightMap[nextIndex] + sedimentMap[nextIndex])
		{
			//Calculate force based on strength of the wind and density of the terrain
			float force = MathsUtils::magnitude3(windVelocity.x, windVelocity.y, windVelocity.z) * (sedimentMap[nextIndex] + heightMap[nextIndex] - height);

			//Abrasion occurs - solid ground is eroded, more sediment is created at this position
			if (sedimentMap[index] <= 0.0f)
			{
				sedimentMap[index] = 0.0f;
				float increment = (dt * abrasion * force * sedimentRate);
				heightMap[index] -= increment;
				sedimentMap[index] += increment;
			}

			else if (sedimentMap[index] > (dt * suspension * force))	//Collided with pile of sediment, particle picks more up and begins sliding
			{
				float increment = (dt * suspension * force);
				sedimentMap[index] -= increment;
				sedimentRate += increment;
				cascade(dt, index, heightMap, sedimentMap, particle, resolution);
			}

			else
			{
				sedimentMap[index] = 0.0f;
			}
		}

		else
		{	//Did not collide with terrain - particle is flying, begin cascading process
			float increment = (dt * suspension * sedimentRate);
			sedimentRate -= increment;
			sedimentMap[index] += (0.5f * increment);
			sedimentMap[nextIndex] += (0.5f * increment);
			cascade(dt, index, heightMap, sedimentMap, particle, resolution);
			cascade(dt, nextIndex, heightMap, sedimentMap, particle, resolution);
		}

		//Strength of wind is too low to be noticeable, break and kill particle
		if (MathsUtils::magnitude3(windVelocity.x, windVelocity.y, windVelocity.z) < 0.01f)
		{
			break;
		}

		index = nextIndex;
	}
}

XMFLOAT3 WindErosion::calculateNormal(int index, int resolution, float* heightMap, float* sedimentMap, float amplitude)
{
	XMFLOAT3 normal = { 0.0f, 0.0f, 0.0f };

	//Indexes of neighbour positions in 4 directions
	int nI[4] =
	{
		index + 1,	//Right along x-axis
		index - 1,	//Left along x-axis
		index + resolution,	//Forward along z-axis
		index - resolution	//Back along z-axis
	};

	for (int m = 0; m < 4; m++)
	{
		//Check if neighbour is within the bounds of the height map, otherwise set to current position index
		if (nI[m] < 0 || nI[m] >= (resolution * resolution) - 1)
			nI[m] = index;
	}

	//Using the indexes to work out gradients with values retrieved from the height map and sediment map
	XMFLOAT3 n0 = { 0.0f, amplitude * (heightMap[nI[0]] - heightMap[index] + sedimentMap[nI[0]] - sedimentMap[index]), 1.0f };
	XMFLOAT3 n1 = { 0.0f, amplitude * (heightMap[nI[1]] - heightMap[index] + sedimentMap[nI[1]] - sedimentMap[index]), -1.0f };
	XMFLOAT3 n2 = { 1.0f, amplitude * (heightMap[nI[2]] - heightMap[index] + sedimentMap[nI[2]] - sedimentMap[index]), 0.0f };
	XMFLOAT3 n3 = { -1.0f, amplitude * (heightMap[nI[3]] - heightMap[index] + sedimentMap[nI[3]] - sedimentMap[index]), 0.0f };

	normal = normal + MathsUtils::crossProduct(n0, n2);
	normal = normal + MathsUtils::crossProduct(n1, n3);
	normal = normal + MathsUtils::crossProduct(n2, n1);
	normal = normal + MathsUtils::crossProduct(n3, n0);

	MathsUtils::normalise3D(normal);

	return normal;
}

XMFLOAT3 WindErosion::calculateDeflectionNormal(WindParticle& particle, int index, int resolution, float* heightMap, float* sedimentMap, float amplitude)
{
	XMFLOAT3 result = { 0.0f, 0.0f, 0.0f };

	XMINT2 p00 = { (int)particle.position.x, (int)particle.position.y };	//Back (floored position)
	XMINT2 p10 = p00 + XMINT2(1, 0);	//Left
	XMINT2 p01 = p00 + XMINT2(0, 1);	//Right
	XMINT2 p11 = p00 + XMINT2(1, 1);	//Front

	//Calculating the normals in each direction, clamped to ensure they are within the bounds of the map
	XMFLOAT3 n00 = calculateNormal(MathsUtils::clamp((p00.x * resolution) + p00.y, 0, (resolution * resolution) - 1), resolution, heightMap, sedimentMap, amplitude);
	XMFLOAT3 n10 = calculateNormal(MathsUtils::clamp((p10.x * resolution) + p10.y, 0, (resolution * resolution) - 1), resolution, heightMap, sedimentMap, amplitude);
	XMFLOAT3 n01 = calculateNormal(MathsUtils::clamp((p01.x * resolution) + p01.y, 0, (resolution * resolution) - 1), resolution, heightMap, sedimentMap, amplitude);
	XMFLOAT3 n11 = calculateNormal(MathsUtils::clamp((p11.x * resolution) + p11.y, 0, (resolution * resolution) - 1), resolution, heightMap, sedimentMap, amplitude);

	//Getting a weighting based on the current floored position
	XMFLOAT2 weights = MathsUtils::modulus(XMFLOAT2((float)p00.x, (float)p00.y), XMFLOAT2(1.0f, 1.0f));
	weights = XMFLOAT2(1.0f - weights.x, 1.0f - weights.y);

	//Scaling each directional normal with respect to the weighting
	n00 = XMFLOAT3(n00.x * weights.x, n00.y * weights.x, n00.z * weights.x);
	n00 = XMFLOAT3(n00.x * weights.y, n00.y * weights.y, n00.z * weights.y);

	n10 = XMFLOAT3(n00.x * (1.0f - weights.x), n10.y * (1.0f - weights.x), n10.z * (1.0f - weights.x));
	n10 = XMFLOAT3(n00.x * weights.y, n10.y * weights.y, n10.z * weights.y);

	n01 = XMFLOAT3(n01.x * weights.x, n01.y * weights.x, n01.z * weights.x);
	n01 = XMFLOAT3(n01.x * (1.0f - weights.y), n01.y * (1.0f - weights.y), n01.z * (1.0f - weights.y));

	n11 = XMFLOAT3(n11.x * (1.0f - weights.x), n11.y * (1.0f - weights.x), n11.z * (1.0f - weights.x));
	n11 = XMFLOAT3(n11.x * (1.0f - weights.y), n11.y * (1.0f - weights.y), n11.z * (1.0f - weights.y));

	//Summing the weighted directions
	result = result + n00;
	result = result + n10;
	result = result + n01;
	result = result + n11;

	return result;
}

void WindErosion::cascade(float dt, int index, float* heightMap, float* sedimentMap, WindParticle& particle, int resolution)
{
	//Neighbour directions (8-Way) stored for ease of use
	const int nX[8] = { -1,-1,-1,0,0,1,1,1 };
	const int nY[8] = { -1,0,1,-1,1,-1,0,1 };
	
	//Neighbour positions
	int neighbours[8] =
	{
		index - resolution - 1,	//Bottom Left
		index - resolution,		//Bottom
		index - resolution + 1,	//Bottom Right
		index - 1,				//Left
		index + 1,				//Right
		index + resolution - 1,	//Top Left
		index + resolution,		//Top
		index + resolution + 1	//Top Right
	};
	
	//Iterate over all neighbours and assess their sediment mass
	for (int m = 0; m < 8; m++)
	{
		//Check if neighbour is within the bounds of the height map, otherwise skip
		if (neighbours[m] < 0 || neighbours[m] >= (resolution * resolution))
			continue;
	
		if (particle.position.x + nX[m] >= resolution || particle.position.y + nY[m] >= resolution)
			continue;
	
		if (particle.position.x + nX[m] < 0 || particle.position.y + nY[m] < 0)
			continue;
	
		//difference in size of the piles
		float difference = (heightMap[index] + sedimentRate) - (heightMap[neighbours[m]] + sedimentRate);
		float excess = abs(difference) - roughness;	//How even or uneven each sediment pile should be
	
		if (excess <= 0)
		{
			continue;
		}
	
		//transfer sediment mass
		float transfer;

		if (difference > 0) //current pile is larger
		{
			transfer = min(sedimentMap[index], excess / 2.0f);
		}
		else         //neighbour pile is larger
		{
			transfer = -min(sedimentMap[neighbours[m]], excess / 2.0f);
		}
	
		sedimentMap[index] -= dt * settling * transfer;
		sedimentMap[neighbours[m]] += dt * settling * transfer;
	}
}
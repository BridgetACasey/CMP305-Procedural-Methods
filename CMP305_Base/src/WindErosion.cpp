#include "WindErosion.h"

#include "MathsUtils.h"

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

void WindErosion::fly(float deltaTime, float amplitude, float* heightMap, float* sedimentMap, WindParticle& particle, int resolution)
{
	int iterations = 0;
	float acceleration = 0.1f;
	int index = ((int)particle.position.y * resolution) + (int)particle.position.x;

	while (true)
	{
		//Follow wind path and alter terrain

		if (height < (heightMap[index] + sedimentMap[index]))
		{
			//Particles underneath the heightmap are moved upwards
			height = (heightMap[index] + sedimentMap[index]);
		}

		else if (height > heightMap[index] + sedimentMap[index])
		{
			//Applying gravity to flying particles
			windVelocity.y -= (deltaTime * acceleration);
		}

		//Accelerate wind
		windVelocity.x += (acceleration * deltaTime * (particle.velocity.x - windVelocity.x));
		windVelocity.y += (acceleration * deltaTime * (particle.velocity.y - windVelocity.y));
		windVelocity.z += (acceleration * deltaTime * (particle.velocity.z - windVelocity.z));

		if (weightedParticles)
		{
			XMFLOAT2 posInc = { deltaTime * windVelocity.x, deltaTime * windVelocity.z };
			XMFLOAT2 direction = { (posInc.x > 0) ? 1.0f : -1.0f, (posInc.y > 0) ? 1.0f : -1.0f };
			XMFLOAT2 weight = calculateParticleWeight(deltaTime, particle, direction, heightMap, index, resolution);

			particle.position.x += weight.x * posInc.x;
			particle.position.y += weight.y * posInc.y;
		}

		else
		{
			particle.position.x += deltaTime * windVelocity.x;
			particle.position.y += deltaTime * windVelocity.z;
		}

		height += (deltaTime * windVelocity.y);

		//The next index position of the particle
		int nextIndex = ((int)particle.position.y * resolution) + (int)particle.position.x;

		//Check that the particle hasn't moved to an out of bounds position
		if (nextIndex < 0 || nextIndex > (resolution * resolution))
		{
			break;
		}

		//Surface Contact
		if (height <= heightMap[nextIndex] + sedimentMap[nextIndex])
		{
			float force = MathsUtils::magnitude1(MathsUtils::magnitude3(windVelocity.x, windVelocity.y, windVelocity.z)*(sedimentMap[nextIndex]+heightMap[nextIndex]-height));

			//Abrasion
			if (sedimentMap[index] <= 0.0f)
			{
				sedimentMap[index] = 0.0f;

				float increment = (deltaTime * abrasion * force * sedimentRate);

				heightMap[index] -= (10000.0f * increment);	//Scaling the increment value so it has a noticeable impact on the height map
				sedimentMap[index] += increment;
			}

			else if (sedimentMap[index] > (deltaTime * suspension * force))
			{
				float increment = (deltaTime * suspension * force);

				sedimentMap[index] -= increment;
				sedimentRate += increment;

				cascade(deltaTime, index, heightMap, sedimentMap, particle, resolution);
			}

			else
			{
				sedimentMap[index] = 0.0f;
			}
		}

		else
		{
			float increment = (deltaTime * suspension * sedimentRate);

			sedimentRate -= increment;

			sedimentMap[index] += (0.5f * increment);
			sedimentMap[nextIndex] += (0.5f * increment);

			cascade(deltaTime, index, heightMap, sedimentMap, particle, resolution);
			cascade(deltaTime, nextIndex, heightMap, sedimentMap, particle, resolution);
		}

		if (MathsUtils::magnitude3(windVelocity.x, windVelocity.y, windVelocity.z) < 0.01f)
		{
			break;
		}

		iterations++;

		index = nextIndex;
	}
}

XMFLOAT2 WindErosion::calculateParticleWeight(float deltaTime, WindParticle& particle, XMFLOAT2 direction, float* heightMap, int index, int resolution)
{
	XMFLOAT3 fwdNeighbours[3] =
	{
		//Nearest neighbour vertex in the direction of the particle
		XMFLOAT3(particle.position.x + direction.x, 0.0f, particle.position.y + direction.y),
		//Vertices immediately adjacent to the particle in the x and z axes respectively
		XMFLOAT3(particle.position.x + direction.x, 0.0f, particle.position.y),
		XMFLOAT3(particle.position.x, 0.0f, particle.position.y + direction.y)
	};

	//Grabbing the position in the y-axis at each vertex from the height map
	for (int i = 0; i < 3; i++)
	{
		if (((fwdNeighbours[i].z * resolution) + fwdNeighbours[i].x) < 0 || ((fwdNeighbours[i].z * resolution) + fwdNeighbours[i].x) >= (resolution * resolution))
			continue;

		fwdNeighbours[i].y = heightMap[(int)((fwdNeighbours[i].z * resolution) + fwdNeighbours[i].x)];
	}

	//Creating vectors to describe the slope of the terrain in each axis
	XMFLOAT3 xDirection = { fwdNeighbours[1].x - particle.position.x, fwdNeighbours[1].y - heightMap[index], fwdNeighbours[1].z - particle.position.y };
	XMFLOAT3 zDirection = { fwdNeighbours[2].x - particle.position.x, fwdNeighbours[2].y - heightMap[index], fwdNeighbours[2].z - particle.position.y };

	MathsUtils::normalise3D(xDirection.x, xDirection.y, xDirection.z);
	MathsUtils::normalise3D(zDirection.x, zDirection.y, zDirection.z);

	//Magnitude of the normalised directional vectors can be used to scale the particle position and steer it down a slope
	float xWeight = MathsUtils::magnitude3(xDirection.x, xDirection.y, xDirection.z);
	float zWeight = MathsUtils::magnitude3(zDirection.x, zDirection.y, zDirection.z);

	return XMFLOAT2(xWeight, zWeight);
}

void WindErosion::cascade(float deltaTime, int index, float* heightMap, float* sedimentMap, WindParticle& particle, int resolution)
{
	//Neighbour directions (8-Way)
	const int nX[8] = { -1,-1,-1,0,0,1,1,1 };
	const int nY[8] = { -1,0,1,-1,1,-1,0,1 };
	
	//Neighbour positions
	int neighbours[8] =
	{
		index - resolution - 1,
		index - resolution,
		index - resolution + 1,
		index - 1,
		index + 1,
		index + resolution - 1,
		index + resolution,
		index + resolution + 1
	};
	
	//Iterate over all neighbours
	for (int m = 0; m < 8; m++)
	{
		//Check if neighbour is within the bounds of the height map, otherwise skip
		if (neighbours[m] < 0 || neighbours[m] >= (resolution * resolution))
			continue;
	
		if (particle.position.x + nX[m] >= resolution || particle.position.y + nY[m] >= resolution)
			continue;
	
		if (particle.position.x + nX[m] < 0 || particle.position.y + nY[m] < 0)
			continue;
	
		//Pile Size Difference
		float difference = (heightMap[index] + sedimentRate) - (heightMap[neighbours[m]] + sedimentRate);
		float excess = abs(difference) - roughness;
	
		if (excess <= 0)
		{
			continue;
		}
	
		//Transfer Mass
		float transfer;
		if (difference > 0) //Pile is Larger
		{
			transfer = min(sedimentMap[index], excess / 2.0);
		}
		else         //Neighbour is Larger
		{
			transfer = -min(sedimentMap[neighbours[m]], excess / 2.0);
		}
	
		sedimentMap[index] -= deltaTime * settling * transfer;
		sedimentMap[neighbours[m]] += deltaTime * settling * transfer;
	}
}
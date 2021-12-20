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

void WindErosion::setWindAttributes(float sed, float sus, float abr, float rgh, float set)
{
	sedimentRate = sed;
	suspension = sus;
	abrasion = abr;
	roughness = rgh;
	settling = set;
}

void WindErosion::fly(float deltaTime, float amplitude, float* heightMap, float* sedimentMap, Particle& particle, int resolution)
{
	int iterations = 0;

	int index = ((int)particle.position.y * resolution) + (int)particle.position.x;

	while (true)
	{
		//Follow "wind path" and alter terrain

		if (height < (heightMap[index] + sedimentMap[index]))
		{
			//Particles underneath the heightmap are moved upwards
			height = (heightMap[index] + sedimentMap[index]);
		}

		else if (height > heightMap[index] + sedimentMap[index])
		{
			//Applying gravity
			windVelocity.y -= (deltaTime * 0.01f);
		}

		//Accelerate wind
		windVelocity.x += (0.1f * deltaTime * (particle.velocity.x - windVelocity.x));
		windVelocity.y += (0.1f * deltaTime * (particle.velocity.y - windVelocity.y));
		windVelocity.z += (0.1f * deltaTime * (particle.velocity.z - windVelocity.z));

		particle.position.x += deltaTime * windVelocity.x;
		particle.position.y += deltaTime * windVelocity.z;

		height += (deltaTime * windVelocity.y);

		//The next index position of the particle
		int nextIndex = ((int)particle.position.y * resolution) + (int)particle.position.x;

		//Check that the particle hasn't moved to an out of bounds position
		if (nextIndex < 0 || nextIndex > ((resolution * resolution) - 1))
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

void WindErosion::cascade(float deltaTime, int index, float* heightMap, float* sedimentMap, Particle& particle, int resolution)
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
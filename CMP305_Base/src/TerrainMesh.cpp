#include "TerrainMesh.h"

TerrainMesh::TerrainMesh( ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution ) :
	PlaneMesh( device, deviceContext, lresolution ) 
{
	Resize( resolution );
	Regenerate( device, deviceContext );

	amplitude = 6.0f;
	frequency = 0.015f;
}

//Cleanup the heightMap
TerrainMesh::~TerrainMesh()
{
	delete[] heightMap;
	heightMap = 0;
	delete[] sedimentMap;
	sedimentMap = 0;
}


//Fill an array of floats that represent the height values at each grid point.
//Here we are producing a Sine wave along the X-axis
void TerrainMesh::BuildHeightMap()
{
	float height = 0.0f;

	//Scale everything so that the look is consistent across terrain resolutions
	const float scale = terrainSize / (float)resolution;

	//TODO: Give some meaning to these magic numbers! What effect does changing them have on terrain?
	for( int j = 0; j < ( resolution ); j++ )
	{
		for( int i = 0; i < ( resolution ); i++ )
		{
			height = (sin((float)i * frequency * scale)) * amplitude;
			height += (sin((float)j * frequency * scale + 1.0f));
			heightMap[(i * resolution) + j] = height;

			sedimentMap[(i * resolution) + j] = 0.0f;
		}
	}	
}

void TerrainMesh::Resize( int newResolution )
{
	resolution = newResolution;

	if (heightMap)
	{
		delete[] heightMap;
	}

	heightMap = new float[resolution * resolution];

	if( vertexBuffer != NULL )
	{
		vertexBuffer->Release();
	}

	vertexBuffer = NULL;

	if (sedimentMap)
	{
		delete[] sedimentMap;
	}

	sedimentMap = new float[resolution * resolution];
}

// Set up the heightmap and create or update the appropriate buffers
void TerrainMesh::Regenerate( ID3D11Device * device, ID3D11DeviceContext * deviceContext )
{
	VertexType* vertices;
	unsigned long* indices;
	int index, i, j;
	float positionX, height, positionZ, u, v, increment;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	
	//Calculate and store the height values
	//BuildHeightMap();

	// Calculate the number of vertices in the terrain mesh.
	// We share vertices in this mesh, so the vertex count is simply the terrain 'resolution'
	// and the index count is the number of resulting triangles * 3 OR the number of quads * 6
	vertexCount = resolution * resolution;

	indexCount = ( ( resolution - 1 ) * ( resolution - 1 ) ) * 6;
	vertices = new VertexType[vertexCount];
	indices = new unsigned long[indexCount];

	index = 0;

	// UV coords.
	u = 0;
	v = 0;
	increment = m_UVscale / resolution;

	//Scale everything so that the look is consistent across terrain resolutions
	const float scale = terrainSize / (float)resolution;

	//Set up vertices
	for( j = 0; j < ( resolution ); j++ ) {
		for( i = 0; i < ( resolution ); i++ ) {
			positionX = (float)i * scale;
			positionZ = (float)( j ) * scale;

			height = heightMap[index];
			vertices[index].position = XMFLOAT3( positionX, height, positionZ );
			vertices[index].texture = XMFLOAT2( u, v );

			u += increment;
			index++;
		}
		u = 0;
		v += increment;
	}

	//Set up index list
	index = 0;
	for( j = 0; j < ( resolution - 1 ); j++ ) {
		for( i = 0; i < ( resolution - 1 ); i++ ) {

			//Build index array
			indices[index] = ( j*resolution ) + i;
			indices[index + 1] = ( ( j + 1 ) * resolution ) + ( i + 1 );
			indices[index + 2] = ( ( j + 1 ) * resolution ) + i;

			indices[index + 3] = ( j * resolution ) + i;
			indices[index + 4] = ( j * resolution ) + ( i + 1 );
			indices[index + 5] = ( ( j + 1 ) * resolution ) + ( i + 1 );
			index += 6;
		}
	}

	//Set up normals
	for( j = 0; j < ( resolution - 1 ); j++ ) {
		for( i = 0; i < ( resolution - 1 ); i++ ) {
			//Calculate the plane normals
			XMFLOAT3 a, b, c;	//Three corner vertices
			a = vertices[j * resolution + i].position;
			b = vertices[j * resolution + i + 1].position;
			c = vertices[( j + 1 ) * resolution + i].position;

			//Two edges
			XMFLOAT3 ab( c.x - a.x, c.y - a.y, c.z - a.z );
			XMFLOAT3 ac( b.x - a.x, b.y - a.y, b.z - a.z );
			
			//Calculate the cross product
			XMFLOAT3 cross;
			cross.x = ab.y * ac.z - ab.z * ac.y;
			cross.y = ab.z * ac.x - ab.x * ac.z;
			cross.z = ab.x * ac.y - ab.y * ac.x;
			float mag = ( cross.x * cross.x ) + ( cross.y * cross.y ) + ( cross.z * cross.z );
			mag = sqrtf( mag );
			cross.x/= mag;
			cross.y /= mag;
			cross.z /= mag;
			vertices[j * resolution + i].normal = cross;
		}
	}

	//Smooth the normals by averaging the normals from the surrounding planes
	XMFLOAT3 smoothedNormal( 0, 1, 0 );
	for( j = 0; j < resolution; j++ ) {
		for( i = 0; i < resolution; i++ ) {
			smoothedNormal.x = 0;
			smoothedNormal.y = 0;
			smoothedNormal.z = 0;
			float count = 0;
			//Left planes
			if( ( i - 1 ) >= 0 ) {
				//Top planes
				if( ( j ) < ( resolution - 1 ) ) {
					smoothedNormal.x += vertices[j * resolution + ( i - 1 )].normal.x;
					smoothedNormal.y += vertices[j * resolution + ( i - 1 )].normal.y;
					smoothedNormal.z += vertices[j * resolution + ( i - 1 )].normal.z;
					count++;
				}
				//Bottom planes
				if( ( j - 1 ) >= 0 ) {
					smoothedNormal.x += vertices[( j - 1 ) * resolution + ( i - 1 )].normal.x;
					smoothedNormal.y += vertices[( j - 1 ) * resolution + ( i - 1 )].normal.y;
					smoothedNormal.z += vertices[( j - 1 ) * resolution + ( i - 1 )].normal.z;
					count++;
				}
			}
			//right planes
			if( ( i ) <( resolution - 1 ) ) {

				//Top planes
				if( ( j ) < ( resolution - 1 ) ) {
					smoothedNormal.x += vertices[j * resolution + i].normal.x;
					smoothedNormal.y += vertices[j * resolution + i].normal.y;
					smoothedNormal.z += vertices[j * resolution + i].normal.z;
					count++;
				}
				//Bottom planes
				if( ( j - 1 ) >= 0 ) {
					smoothedNormal.x += vertices[( j - 1 ) * resolution + i].normal.x;
					smoothedNormal.y += vertices[( j - 1 ) * resolution + i].normal.y;
					smoothedNormal.z += vertices[( j - 1 ) * resolution + i].normal.z;
					count++;
				}
			}
			smoothedNormal.x /= count;
			smoothedNormal.y /= count;
			smoothedNormal.z /= count;

			float mag = sqrt( ( smoothedNormal.x * smoothedNormal.x ) + ( smoothedNormal.y * smoothedNormal.y ) + ( smoothedNormal.z * smoothedNormal.z ) );
			smoothedNormal.x /= mag;
			smoothedNormal.y /= mag;
			smoothedNormal.z /= mag;

			vertices[j * resolution + i].normal = smoothedNormal;
		}
	}
	//If we've not yet created our dyanmic Vertex and Index buffers, do that now
	if( vertexBuffer == NULL ) {
		CreateBuffers( device, vertices, indices );
	}
	else {
		//If we've already made our buffers, update the information
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory( &mappedResource, sizeof( D3D11_MAPPED_SUBRESOURCE ) );

		//  Disable GPU access to the vertex buffer data.
		deviceContext->Map( vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
		//  Update the vertex buffer here.
		memcpy( mappedResource.pData, vertices, sizeof( VertexType ) * vertexCount );
		//  Reenable GPU access to the vertex buffer data.
		deviceContext->Unmap( vertexBuffer, 0 );
	}

	// Release the arrays now that the buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;
	delete[] indices;
	indices = 0;
}

void TerrainMesh::renderSampleTerrain(float dt)
{
	flatten();

	amplitude = 28.0f;
	frequency = 0.033f;

	generateRidgedFBM(8, 0.4f, 1.2f);

	frequency = 0.015f;

	generateFBM(8, 0.5f, 1.1f);

	//float pVel[3] = { 1.0f, 0.0f, 1.0f };
	//float wVel[3] = { 1.0f, -1.0f, 1.0f };
	//windErosion(dt, 5000, pVel, wVel, 0.7f, 0.025f, 0.5f, 0.01f, 0.5f, true);
}

void TerrainMesh::flatten()
{
	for (int j = 0; j < (resolution); j++)
	{
		for (int i = 0; i < (resolution); i++)
		{
			heightMap[(i * resolution) + j] = 0.0f;
		}
	}
}

void TerrainMesh::invert()
{
	for (int j = 0; j < (resolution); j++)
	{
		for (int i = 0; i < (resolution); i++)
		{
			heightMap[(i * resolution) + j] *= -1.0f;
		}
	}
}

void TerrainMesh::random()
{
	float height = 0.0f;

	for (int j = 0; j < (resolution); j++)
	{
		for (int i = 0; i < (resolution); i++)
		{
			heightMap[(j * resolution) + i] = (float)((rand() % (int)amplitude) + 1);
		}
	}
}

void TerrainMesh::smooth(int itr)
{
	for (int k = 0; k < itr; k++)
	{
		float height = 0.0f;
		float finalHeight = 0.0f;
		int neighbours = 0;
		int MAX_BOUND = resolution * resolution;
		int currentVertex = 0;

		for (int j = 0; j < (resolution); j++)
		{
			for (int i = 0; i < (resolution); i++)
			{
				currentVertex = (j * resolution) + i;
				neighbours = 0;
				finalHeight = 0.0f;
				height = 0.0f;

				if ((currentVertex)+1 < MAX_BOUND)
				{
					height += heightMap[((j * resolution) + i) + 1];
					neighbours++;
				}

				if ((currentVertex)-1 > 0)
				{
					height += heightMap[currentVertex - 1];
					neighbours++;
				}

				if ((currentVertex)+resolution < MAX_BOUND)
				{
					height += heightMap[currentVertex + resolution];
					neighbours++;
				}

				if ((currentVertex)-resolution > 0)
				{
					height += heightMap[currentVertex - resolution];
					neighbours++;
				}

				finalHeight = (height) / (float)neighbours;
				heightMap[currentVertex] = finalHeight;
			}
		}
	}
}

void TerrainMesh::fault(int itr)
{
	float faultFactor = 1.0f / itr;
	float faultMultiplier = 1.0f + faultFactor;

	for (int k = 0; k < itr; k++)
	{
		float startPoint[2] = { (rand() % resolution), (rand() % resolution) };	//P1
		float endPoint[2] = { (rand() % resolution), (rand() % resolution) };	//P2

		float nextPoint[2] = { 0.0f, 0.0f };	//The coordinates of the next vertex to check

		XMFLOAT3 lineOne = { endPoint[0] - startPoint[0], endPoint[1] - startPoint[1], 0.0f };	//L1
		XMFLOAT3 lineTwo = { 0.0f, 0.0f, 0.0f };	//L2
		XMFLOAT3 cross{};	//To store the result of the cross product

		for (int j = 0; j < (resolution); j++)
		{
			nextPoint[0] = (float)j;

			for (int i = 0; i < (resolution); i++)
			{
				nextPoint[1] = (float)i;

				lineTwo = { startPoint[0] - nextPoint[0], startPoint[1] - nextPoint[1], 0.0f };

				XMStoreFloat3(&cross, XMVector3Cross(XMLoadFloat3(&lineOne), XMLoadFloat3(&lineTwo)));

				if (cross.z > 0.0f)
				{
					heightMap[(j * resolution) + i] += (amplitude * faultMultiplier);
				}

				else if (cross.z <= 0.0f)
				{
					heightMap[(j * resolution) + i] -= (amplitude * faultMultiplier);
				}
			}
		}

		faultMultiplier -= faultFactor;
	}
}

void TerrainMesh::perlinOriginal()
{
	for (int j = 0; j < (resolution); j++)
	{
		for (int i = 0; i < (resolution); i++)
		{
			float height = heightMap[(j * resolution) + i];

			float x = (float)i * frequency;	//Scaling the input for noise
			float y = (float)j * frequency;

			height += noise.generatePerlin2D(x, y) * amplitude;

			heightMap[(j * resolution) + i] = height;
		}
	}
}

void TerrainMesh::perlinImproved()
{
	for (int j = 0; j < (resolution); j++)
	{
		for (int i = 0; i < (resolution); i++)
		{
			float height = heightMap[(j * resolution) + i];

			float x = (float)i * frequency;	//Scaling the input for noise
			float y = (float)j * frequency;

			height += noise.generateImprovedPerlin(x, y) * amplitude;

			heightMap[(j * resolution) + i] = height;
		}
	}
}

void TerrainMesh::generateFBM(int octaves, float ampl, float freq)
{
	float height = 0.0f;
	float x = 0.0f;
	float y = 0.0f;

	float a = amplitude;
	float f = frequency;

	for (int o = 0; o < octaves; o++)
	{
		for (int j = 0; j < (resolution); j++)
		{
			for (int i = 0; i < (resolution); i++)
			{
				height = heightMap[(j * resolution) + i];

				x = (float)i * f;
				y = (float)j * f;

				//height += noise.generatePerlin2D(x, y) * a;
				height += noise.generateImprovedPerlin(x, y) * a;

				heightMap[(j * resolution) + i] = height;
			}
		}

		a *= ampl;
		f *= freq;
	}
}

void TerrainMesh::generateRidgedFBM(int octaves, float ampl, float freq)
{
	float height = 0.0f;
	float x = 0.0f;
	float y = 0.0f;

	float a = amplitude;
	float f = frequency;

	for (int o = 0; o < octaves; o++)
	{
		for (int j = 0; j < (resolution); j++)
		{
			for (int i = 0; i < (resolution); i++)
			{
				height = heightMap[(j * resolution) + i];

				x = (float)i * f;
				y = (float)j * f;

				//height -= noise.generatePerlin2D(x, y) * a;
				height -= noise.generateImprovedPerlin(x, y) * a;

				if (height < (-a))	//Negative amplitude represents the lowest point of the height map
				{
					height = sqrtf(height * height);
				}

				heightMap[(j * resolution) + i] = height;
			}
		}

		a *= ampl;
		f *= freq;
	}

	invert();
}

void TerrainMesh::windErosion(float dt, int itr, float* pVel, float* wVel, float sed, float sus, float abr, float rgh, float set, bool weigh)
{
	for (int j = 0; j < itr; j++)
	{
		WindParticle originParticle;
		int index = 0;
		//Spawn new particles on a boundary
		int shift = rand() % (resolution + resolution);

		if (shift < resolution)	//Spawn along x boundary
		{
			originParticle.position.x = shift;

			if (originParticle.velocity.z > 0.0f)
			originParticle.position.y = 0;
			else
			originParticle.position.y = (resolution - 1);
		}

		else //Spawn along z boundary
		{
			originParticle.position.y = shift - resolution;

			if (originParticle.velocity.x > 0.0f)
				originParticle.position.x = 0;
			else
				originParticle.position.x = (resolution - 1);
		}

		originParticle.velocity.x = pVel[0];
		originParticle.velocity.y = pVel[1];
		originParticle.velocity.z = pVel[2];

		//Keeping the edge of the terrain constant to prevent particles from slipping and creating pits
		index = (int)((originParticle.position.y * resolution) + originParticle.position.x);
		float edgeHeight = heightMap[index];

		WindErosion wind(wVel[0], wVel[1], wVel[2]);
		wind.setWindAttributes(sed, sus, abr, rgh, set, weigh);
		wind.fly(dt, amplitude, heightMap, sedimentMap, originParticle, resolution);

		heightMap[index] = edgeHeight;
	}
}

//Create the vertex and index buffers that will be passed along to the graphics card for rendering
//For CMP305, you don't need to worry so much about how or why yet, but notice the Vertex buffer is DYNAMIC here as we are changing the values often
void TerrainMesh::CreateBuffers( ID3D11Device* device, VertexType* vertices, unsigned long* indices ) {

	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Set up the description of the dyanmic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof( VertexType ) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer( &vertexBufferDesc, &vertexData, &vertexBuffer );

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof( unsigned long ) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	device->CreateBuffer( &indexBufferDesc, &indexData, &indexBuffer );
}
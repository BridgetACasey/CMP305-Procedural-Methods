// Light pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)

Texture2D texture0 : register(t0);
Texture2D texture1 : register(t1);
SamplerState sampler0 : register(s0);

cbuffer LightBuffer : register(b0)
{
    float4 diffuse;
	float3 direction;
	float amplitude;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
	float intensity = saturate(dot(normal, lightDirection));
	float4 colour = saturate(diffuse * intensity);
	return colour;
}

float4 main(InputType input) : SV_TARGET
{
    float4 lightColour, textureColour, woodTexture;

	// Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour.
    lightColour = calculateLighting(-direction, input.normal, diffuse);
	textureColour = texture0.Sample(sampler0, input.tex);
    woodTexture = texture1.Sample(sampler0, input.tex);
	
    float height = input.worldPosition.y;
	
	//Calculate a scalar value to blend the texture colours based on the height of the terrain
    float heightMultiplier = saturate(height / amplitude);
	
    textureColour *= float4((1.0f - heightMultiplier), (1.0f - heightMultiplier), (1.0f - heightMultiplier), 1.0f);
    woodTexture *= float4(heightMultiplier, heightMultiplier, heightMultiplier, 1.0f);

    return lightColour * (textureColour + woodTexture);
}




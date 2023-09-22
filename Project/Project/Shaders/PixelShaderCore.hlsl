#include "ShaderCore.hlsl"


#define MAX_INSTANCES 5
#define MAX_DIRECTIONAL_LIGHTS 3
#define MAX_POINT_LIGHTS 3
#define MAX_SPOT_LIGHTS 3


struct DirectionalLight
{
	float4 Color;
	float4 Direction;
};

struct PointLight
{
	float4 Color;
	float4 Position;
	float Range;
	float3 Attenuation;
};

struct SpotLight
{
	float4 Color;
	float4 Position;
	float4 Direction;
	float Range;
	float3 Attenuation;
	float Cone;
};


cbuffer PixelConstantBuffer : register(b1)
{
	float4 AmbientColor;
	float4 InstanceColors[MAX_INSTANCES];
	DirectionalLight DirectionalLights[MAX_DIRECTIONAL_LIGHTS];
	PointLight PointLights[MAX_POINT_LIGHTS];
	SpotLight SpotLights[MAX_SPOT_LIGHTS];
	float Time;
	float3 Padding;
}


Texture2D DiffuxeTexture2D : register(t0);
TextureCube DiffuseTextureCube : register(t1);
SamplerState LinearSampler : register(s0);


float4 ApplyAmbientToPixel(float4 pixelColor, float4 textureColor)
{
	pixelColor = saturate(pixelColor + (AmbientColor * textureColor));
	pixelColor.a = 1;
	return pixelColor;
}

float4 ApplyDirectionalLightsToPixel(float4 pixelColor, ShaderVertex vertex)
{
	for (unsigned int i = 0; i < MAX_DIRECTIONAL_LIGHTS; ++i)
	{
		DirectionalLight light = DirectionalLights[i];
		pixelColor += saturate(dot((float3) light.Direction, vertex.Normal) * light.Color);
	}
	return pixelColor;
}

float4 ApplyPointLightsToPixel(float4 pixelColor, float4 textureColor, ShaderVertex vertex)
{
	for (unsigned int i = 0; i < MAX_POINT_LIGHTS; ++i)
	{
		PointLight light = PointLights[i];
		float3 lightToVertexVector = light.Position.xyz - vertex.WorldPosition.xyz;
		float distance = length(lightToVertexVector);
		if (distance <= light.Range)
		{
			lightToVertexVector /= distance;
			float lightIntensity = dot(lightToVertexVector, vertex.Normal);
			if (lightIntensity > 0)
			{
				pixelColor += lightIntensity * textureColor * light.Color;
				pixelColor /= light.Attenuation[0]
					+ (light.Attenuation[1] * distance)
					+ (light.Attenuation[2] * (distance * distance));
			}
		}
	}
	return pixelColor;
}

float4 ApplySpotLightsToPixel(float4 pixelColor, ShaderVertex vertex)
{
	return pixelColor;
}

float4 ApplyLightsToPixel(float4 pixelColor, float4 textureColor, ShaderVertex vertex)
{
	pixelColor = ApplyPointLightsToPixel(pixelColor, textureColor, vertex);
	pixelColor = ApplyDirectionalLightsToPixel(pixelColor, vertex);
	pixelColor = ApplySpotLightsToPixel(pixelColor, vertex);
	pixelColor = ApplyAmbientToPixel(pixelColor, textureColor);
	return pixelColor;
}

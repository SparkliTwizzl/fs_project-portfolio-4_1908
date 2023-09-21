#include "ShaderCore.hlsl"


cbuffer VertexConstantBuffer : register(b0)
{
	matrix WorldMatrix;
	matrix ViewMatrix;
	matrix ProjectionMatrix;
	matrix InstanceOffsets[5];
	float Time;
	float3 Padding;
}

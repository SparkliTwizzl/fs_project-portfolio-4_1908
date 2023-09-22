#include "ShaderCore.hlsl"


cbuffer VertexConstantBuffer : register(b0)
{
	matrix WorldMatrix;
	matrix ViewMatrix;
	matrix ProjectionMatrix;
	matrix InstanceOffsets[5];
	float Time;
}


float4 ApplyCameraMatricesToPosition(float4 position, matrix world, matrix view, matrix projection)
{
	position = mul(position, world);
	position = mul(position, view);
	position = mul(position, projection);
	return position;
}

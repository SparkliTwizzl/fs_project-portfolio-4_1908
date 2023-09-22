#include "GeometryShaderCore.hlsl"


[maxvertexcount(6)]
void main(triangle ShaderVertex input[3], inout TriangleStream<ShaderVertex> outputStream)
{
	float3 offsets[2] =
	{
		float3(-1, 0, 0),
		float3(1, 0, 0),
	};

	ShaderVertex output = (ShaderVertex) 0;
	for (uint i = 0; i < 6; ++i)
	{
		uint vertexIndex = i % 3;
		uint offsetIndex = i / 3;
		output = input[vertexIndex];
		output.Position += float4(offsets[offsetIndex], 0);
		output.WorldPosition += float4(offsets[offsetIndex], 0);
		outputStream.Append(output);
	}

	outputStream.RestartStrip();
}
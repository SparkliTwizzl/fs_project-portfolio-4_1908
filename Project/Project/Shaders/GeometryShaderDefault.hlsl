#include "GeometryShaderCore.hlsl"


[maxvertexcount(3)]
void main(triangle ShaderVertex input[3], inout TriangleStream<ShaderVertex> outputStream)
{
	ShaderVertex output = (ShaderVertex) 0;

	float3 vertexOffsets[3] =
	{
		float3(1, 0, 0),
		float3(0, 1, 0),
		float3(0, 0, 1)
	};

	for (uint i = 0; i < 3; ++i)
	{
		output.Color = input[i].Color;
		output.InstanceId = input[i].InstanceId;
		output.Normal = input[i].Normal;
		output.Position = input[i].Position;
		output.Texel = input[i].Texel;
		output.WorldPosition = input[i].WorldPosition;

		outputStream.Append(output);
	}

	outputStream.RestartStrip();
}

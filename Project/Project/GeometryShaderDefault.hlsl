#include "GeometryShaderCore.hlsllib"


[maxvertexcount(3)]
void main(triangle ShaderInput input[3], inout TriangleStream<ShaderOutput> outputStream)
{
	ShaderOutput output = (ShaderOutput) 0;

	float3 vertexOffsets[3] =
	{
		float3(1, 0, 0),
		float3(0, 1, 0),
		float3(0, 0, 1)
	};

	for (uint i = 0; i < 3; ++i)
	{
		output.Position = input[i].Position;
		output.Normal = input[i].Normal;
		output.Texel = input[i].Texel;
		output.Color = input[i].Color;
		output.InstanceId = input[i].InstanceId;
		output.WorldPosition = input[i].WorldPosition;

		outputStream.Append(output);
	}

	outputStream.RestartStrip();
}

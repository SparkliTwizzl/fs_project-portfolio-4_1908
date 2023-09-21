#include "GeometryShaderCore.hlsl"


[maxvertexcount(6)]
void main(triangle ShaderVertex input[3], inout TriangleStream<ShaderVertex> outputStream)
{
	ShaderVertex output = (ShaderVertex) 0;

	float3 offsets[2] =
	{
		float3(-1, 0, 0),
		float3(1, 0, 0)
	};

	for (uint i = 0; i < 6; ++i)
	{
		if (i < 3)
		{
			output.Color = input[i].Color;
			output.InstanceId = input[i].InstanceId;
			output.Normal = input[i].Normal;
			output.Position = input[i].Position + float4(offsets[0], 0);
			output.Texel = input[i].Texel;
			output.WorldPosition = input[i].WorldPosition + float4(offsets[0], 0);
		}
		else
		{
			uint index = i - 3;
			output.Color = input[index].Color;
			output.InstanceId = input[index].InstanceId;
			output.Normal = input[index].Normal;
			output.Position = input[index].Position + float4(offsets[1], 0);
			output.Texel = input[index].Texel;
			output.WorldPosition = input[index].WorldPosition + float4(offsets[1], 0);
		}

		outputStream.Append(output);
	}

	outputStream.RestartStrip();
}
#include "VertexShaderCore.hlsl"


ShaderVertex main(RawVertex input)
{
	ShaderVertex output = (ShaderVertex) 0;
	output.Position = input.Position;
	output.Position.x = sin(input.Position.x * Time);
	output.Position.y = sin(input.Position.y * Time);
	output.Position.z = sin(input.Position.z * Time);
	output.Position = mul(output.Position, InstanceOffsets[input.InstanceId]);
	output.Position = mul(output.Position, WorldMatrix);
	output.WorldPosition = output.Position;
	output.Position = mul(output.Position, ViewMatrix);
	output.Position = mul(output.Position, ProjectionMatrix);

	//output.Normal = input.Normal;
	output.Normal = mul(float4(input.Normal, 0), WorldMatrix).xyz;

	output.Texel = input.Texel;
	output.Color = input.Color;
	output.InstanceId = input.InstanceId;

	return output;
}
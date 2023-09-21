#include "VertexShaderCore.hlsl"


ShaderVertex main(RawVertex input)
{
	ShaderVertex output = (ShaderVertex) 0;

	output.Color = input.Color;
	output.InstanceId = input.InstanceId;
	output.Normal = mul(float4(input.Normal, 0), WorldMatrix).xyz;
	output.Position.x = sin(input.Position.x * Time);
	output.Position.y = sin(input.Position.y * Time);
	output.Position.z = sin(input.Position.z * Time);
	output.Position = mul(output.Position, InstanceOffsets[input.InstanceId]);
	output.Position = mul(output.Position, WorldMatrix);
	output.Position = mul(output.Position, ViewMatrix);
	output.Position = mul(output.Position, ProjectionMatrix);
	output.Texel = input.Texel;
	output.WorldPosition.x = sin(input.Position.x * Time);
	output.WorldPosition.y = sin(input.Position.y * Time);
	output.WorldPosition.z = sin(input.Position.z * Time);
	output.WorldPosition = mul(output.WorldPosition, InstanceOffsets[input.InstanceId]);
	output.WorldPosition = mul(output.WorldPosition, WorldMatrix);

	return output;
}
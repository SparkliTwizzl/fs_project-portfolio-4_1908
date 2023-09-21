#include "VertexShaderCore.hlsllib"


ShaderOutput main(ShaderInput input)
{
	ShaderOutput output = (ShaderOutput) 0;
	output.Position = mul(input.Position, InstanceOffsets[input.InstanceId]);
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

#include "VertexShaderCore.hlsl"


ShaderVertex main(RawVertex input)
{
	ShaderVertex output = (ShaderVertex) 0;
	float4 position = mul(input.Position, InstanceOffsets[input.InstanceId]);

	output.Color = input.Color;
	output.InstanceId = input.InstanceId;
	output.Normal = mul(float4(input.Normal, 0), WorldMatrix).xyz;
	output.Position = ApplyCameraMatricesToPosition(position, WorldMatrix, ViewMatrix, ProjectionMatrix);
	output.Texel = input.Texel;
	output.WorldPosition = mul(position, WorldMatrix);

	return output;
}

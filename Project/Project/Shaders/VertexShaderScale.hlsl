#include "VertexShaderCore.hlsl"


ShaderVertex main(RawVertex input)
{
	ShaderVertex output = (ShaderVertex) 0;
	float modifier = abs(sin(Time)) + 0.5f;
	float4 distortedPosition = input.Position * modifier;
	distortedPosition.w = input.Position.w;
	distortedPosition = mul(distortedPosition, InstanceOffsets[input.InstanceId]);

	output.Color = input.Color;
	output.InstanceId = input.InstanceId;
	output.Normal = mul(float4(input.Normal, 0), WorldMatrix).xyz;
	output.Position = ApplyCameraMatricesToPosition(distortedPosition, WorldMatrix, ViewMatrix, ProjectionMatrix);
	output.Texel = input.Texel;
	output.WorldPosition = mul(distortedPosition, WorldMatrix);

	return output;
}
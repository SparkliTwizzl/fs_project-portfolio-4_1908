#include "PixelShaderCore.hlsl"


float4 main(ShaderVertex input) : SV_TARGET
{
	input.Normal = normalize(input.Normal);
	return ApplyLightsToPixel(float4(0, 0, 0, 0), InstanceColors[input.InstanceId], input);
}

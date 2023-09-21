#include "PixelShaderCore.hlsllib"


float4 main(ShaderInput input) : SV_TARGET
{
	float4 finalColor = InstanceColors[input.InstanceId];
	finalColor.a = 1;
	return finalColor;
}
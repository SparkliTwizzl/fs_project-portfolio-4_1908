#include "PixelShaderCore.hlsllib"


float4 main(ShaderVertex input) : SV_TARGET
{
	float4 finalColor = InstanceColors[input.InstanceId];
	finalColor.a = 1;
	return finalColor;
}
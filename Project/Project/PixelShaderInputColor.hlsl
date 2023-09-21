#include "PixelShaderCore.hlsllib"


float4 main(ShaderInput input) : SV_TARGET
{
	float4 finalColor = input.Color;
	finalColor.a = 1;
	return finalColor;
}
#include "PixelShaderCore.hlsl"


float4 main(ShaderVertex input) : SV_TARGET
{
	float4 finalColor = input.Color;
	finalColor.a = 1;
	return finalColor;
}
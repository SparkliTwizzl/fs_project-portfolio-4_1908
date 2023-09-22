#include "PixelShaderCore.hlsl"


float4 main(ShaderVertex input) : SV_TARGET
{
	return input.Color;
}
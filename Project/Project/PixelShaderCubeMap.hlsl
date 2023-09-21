#include "PixelShaderCore.hlsllib"


float4 main(ShaderVertex input) : SV_TARGET
{
	float4 finalColor = DiffuseTextureCube.Sample(LinearSampler, input.Texel);
	finalColor.a = 1;
	return finalColor;
}
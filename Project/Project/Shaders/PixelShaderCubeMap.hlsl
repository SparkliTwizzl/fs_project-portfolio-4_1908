#include "PixelShaderCore.hlsl"


float4 main(ShaderVertex input) : SV_TARGET
{
	return DiffuseTextureCube.Sample(LinearSampler, input.Texel);
}
#include "PixelShaderCore.hlsl"


float4 main(ShaderVertex input) : SV_TARGET
{
	input.Normal = abs(normalize(input.Normal));
	return float4(input.Normal.xyz, 1);
}

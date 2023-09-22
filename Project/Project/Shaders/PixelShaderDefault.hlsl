#include "PixelShaderCore.hlsl"


float4 main(ShaderVertex input) : SV_TARGET
{
	input.Normal = normalize(input.Normal);
	float4 textureColor = DiffuxeTexture2D.Sample(LinearSampler, input.Texel.xy);
	return ApplyLightsToPixel(float4(0, 0, 0, 0), textureColor, input);
}

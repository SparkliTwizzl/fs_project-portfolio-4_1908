#include "PixelShaderCore.hlsl"


float4 main(ShaderVertex input) : SV_TARGET
{
	input.Normal = normalize(input.Normal);
	float2 texel = input.Texel.xy;
	texel.x *= sin(texel.y * Time);
	texel.y *= cos(texel.x * Time);
	float4 textureColor = DiffuxeTexture2D.Sample(LinearSampler, texel);
	return ApplyLightsToPixel(float4(0, 0, 0, 0), textureColor, input);
}

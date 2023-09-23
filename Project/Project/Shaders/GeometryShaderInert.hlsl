#include "GeometryShaderCore.hlsl"


[maxvertexcount(3)]
void main(triangle ShaderVertex input[3], inout TriangleStream<ShaderVertex> outputStream)
{
	for (uint i = 0; i < 3; ++i)
	{
		outputStream.Append(input[i]);
	}
}

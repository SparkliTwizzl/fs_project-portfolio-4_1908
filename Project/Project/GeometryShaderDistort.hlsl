struct ShaderInput
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float3 Texel : TEXCOORD;
	float4 Color : COLOR;
	uint InstanceId : SV_INSTANCEID;
	float4 WorldPosition : WORLDPOSITION;
};

struct ShaderOutput
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float3 Texel : TEXCOORD;
	float4 Color : COLOR;
	uint InstanceId : SV_INSTANCEID;
	float4 WorldPosition : WORLDPOSITION;
};


[maxvertexcount(6)]
void main(triangle ShaderInput input[3], inout TriangleStream<ShaderOutput> outputStream)
{
	ShaderOutput output = (ShaderOutput) 0;

	float3 offsets[2] =
	{
		float3(-1, 0, 0),
		float3(1, 0, 0)
	};

	for (uint i = 0; i < 6; ++i)
	{
		if (i < 3)
		{
			output.Position = input[i].Position;
			output.Position = output.Position + float4(offsets[0], 0);
			output.Normal = input[i].Normal;
			output.Texel = input[i].Texel;
			output.Color = input[i].Color;
			output.InstanceId = input[i].InstanceId;
			output.WorldPosition = input[i].WorldPosition + float4(offsets[0], 0);
		}
		else
		{
			output.Position = input[i - 3].Position;
			output.Position = output.Position + float4(offsets[1], 0);
			output.Normal = input[i - 3].Normal;
			output.Texel = input[i - 3].Texel;
			output.Color = input[i - 3].Color;
			output.InstanceId = input[i - 3].InstanceId;
			output.WorldPosition = input[i - 3].WorldPosition + float4(offsets[1], 0);
		}

		outputStream.Append(output);
	}

	outputStream.RestartStrip();
}
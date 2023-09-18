struct InputData
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float3 Texel : TEXCOORD;
	float4 Color : COLOR;
	uint InstanceId : SV_INSTANCEID;
	float4 WorldPosition : WORLDPOSITION;
};

struct OutputData
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float3 Texel : TEXCOORD;
	float4 Color : COLOR;
	uint InstanceId : SV_INSTANCEID;
	float4 WorldPosition : WORLDPOSITION;
};

[maxvertexcount(3)]
void main(triangle InputData input[3], inout TriangleStream<OutputData> outputStream)
{
	OutputData output = (OutputData) 0;

	float3 vertexOffsets[3] =
	{
		float3(1, 0, 0),
		float3(0, 1, 0),
		float3(0, 0, 1)
	};

	for (uint i = 0; i < 3; i++)
	{
		output.Position = input[i].Position;
		output.Normal = input[i].Normal;
		output.Texel = input[i].Texel;
		output.Color = input[i].Color;
		output.InstanceId = input[i].InstanceId;
		output.WorldPosition = input[i].WorldPosition;

		outputStream.Append(output);
	}

	outputStream.RestartStrip();
}

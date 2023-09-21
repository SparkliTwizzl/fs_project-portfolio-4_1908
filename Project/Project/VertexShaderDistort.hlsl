#pragma pack_matrix(row_major)


struct ShaderInput
{
	float4 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Texel : TEXCOORD;
	float4 Color : COLOR;
	uint InstanceId : SV_INSTANCEID;
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

cbuffer ConstantBuffer : register(b0)
{
	matrix WorldMatrix;
	matrix ViewMatrix;
	matrix ProjectionMatrix;
	matrix InstanceOffsets[5];
	float Time;
	float3 Padding;
}


ShaderOutput main(ShaderInput input)
{
	ShaderOutput output = (ShaderOutput) 0;
	output.Position = input.Position;
	output.Position.x = sin(input.Position.x * Time);
	output.Position.y = sin(input.Position.y * Time);
	output.Position.z = sin(input.Position.z * Time);
	output.Position = mul(output.Position, InstanceOffsets[input.InstanceId]);
	output.Position = mul(output.Position, WorldMatrix);
	output.WorldPosition = output.Position;
	output.Position = mul(output.Position, ViewMatrix);
	output.Position = mul(output.Position, ProjectionMatrix);

	//output.Normal = input.Normal;
	output.Normal = mul(float4(input.Normal, 0), WorldMatrix).xyz;

	output.Texel = input.Texel;
	output.Color = input.Color;
	output.InstanceId = input.InstanceId;

	return output;
}
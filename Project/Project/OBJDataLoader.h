#pragma once

#include "DataTypes.h"


struct OBJVertex
{
	float3 Position;
	float3 Texel;
	float3 Normal;
};

struct OBJMesh
{
	OBJVertex* Vertices;
	unsigned int VertexCount;
	unsigned int* Indices;
	unsigned int IndexCount;
};

OBJMesh LoadOBJMesh(const char* filePath);

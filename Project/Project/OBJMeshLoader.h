#pragma once

#include "DataTypes.h"


struct OBJVertex
{
	float3 Normal;
	float3 Position;
	float3 Texel;
};

struct OBJMesh
{
	unsigned int* Indices;
	unsigned int IndexCount;
	OBJVertex* Vertices;
	unsigned int VertexCount;
};


class OBJMeshLoader
{
public:
	OBJMesh LoadOBJMesh(const char* filePath);
};

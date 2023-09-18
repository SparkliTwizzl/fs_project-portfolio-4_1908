#pragma once

struct OBJVertex
{
	float Position[3];
	float Texel[3];
	float Normal[3];
};

struct OBJMesh
{
	OBJVertex* Vertices;
	unsigned int VertexCount;
	unsigned int* Indices;
	unsigned int IndexCount;
};

OBJMesh LoadOBJMesh(const char* filePath);

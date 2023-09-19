#include "OBJDataLoader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using std::vector;


struct UnstructuredVertex
{
	unsigned int PositionIndex;
	unsigned int TexelIndex;
	unsigned int NormalIndex;
};

struct OBJTriangle
{
	UnstructuredVertex Vertices[3];
};

struct UnstructuredMeshData
{
	vector<float3> Positions;
	vector<float3> Texels;
	vector<float3> Normals;
	vector<OBJTriangle> Faces;
};

struct CompactifiedMeshData
{
	vector<UnstructuredVertex> Vertices;
	vector<unsigned int> Indices;
};

struct NormalizedMeshData
{
	OBJVertex* Vertices;
	unsigned int* Indices;
};


CompactifiedMeshData CompactifyUnstructuredMeshData(UnstructuredMeshData unstructuredMeshData)
{
	CompactifiedMeshData result = {};
	for (unsigned int faceIndex = 0; faceIndex < unstructuredMeshData.Faces.size(); faceIndex++)
	{
		for (unsigned int faceVertexIndex = 0; faceVertexIndex < 3; faceVertexIndex++)
		{
			bool isUnique = true;
			unsigned int index = 0;
			for (unsigned int unsortedVertexIndex = 0; unsortedVertexIndex < result.Vertices.size(); ++unsortedVertexIndex)
			{

				if (unstructuredMeshData.Faces[faceIndex].Vertices[faceVertexIndex].PositionIndex == result.Vertices[unsortedVertexIndex].PositionIndex
					&& unstructuredMeshData.Faces[faceIndex].Vertices[faceVertexIndex].TexelIndex == result.Vertices[unsortedVertexIndex].TexelIndex
					&& unstructuredMeshData.Faces[faceIndex].Vertices[faceVertexIndex].NormalIndex == result.Vertices[unsortedVertexIndex].NormalIndex)
				{
					isUnique = false;
					index = unsortedVertexIndex;
					break;
				}
			}

			if (isUnique)
			{
				index = result.Vertices.size();
				result.Vertices.push_back(unstructuredMeshData.Faces[faceIndex].Vertices[faceVertexIndex]);
			}

			result.Indices.push_back(index);
		}
	}

	return result;
}

NormalizedMeshData NormalizeStructuredMeshData(UnstructuredMeshData unstructuredMeshData, CompactifiedMeshData compactifiedMeshData)
{
	NormalizedMeshData result = {};

	result.Vertices = new OBJVertex[compactifiedMeshData.Vertices.size()];
	for (unsigned int i = 0; i < compactifiedMeshData.Vertices.size(); ++i)
	{
		OBJVertex vertex = {};
		vertex.Position.x = unstructuredMeshData.Positions[compactifiedMeshData.Vertices[i].PositionIndex].x;
		vertex.Position.y = unstructuredMeshData.Positions[compactifiedMeshData.Vertices[i].PositionIndex].y;
		vertex.Position.z = unstructuredMeshData.Positions[compactifiedMeshData.Vertices[i].PositionIndex].z;
		vertex.Texel.x = unstructuredMeshData.Texels[compactifiedMeshData.Vertices[i].TexelIndex].x;
		vertex.Texel.y = unstructuredMeshData.Texels[compactifiedMeshData.Vertices[i].TexelIndex].y;
		vertex.Texel.z = unstructuredMeshData.Texels[compactifiedMeshData.Vertices[i].TexelIndex].z;
		vertex.Normal.x = unstructuredMeshData.Normals[compactifiedMeshData.Vertices[i].NormalIndex].x;
		vertex.Normal.y = unstructuredMeshData.Normals[compactifiedMeshData.Vertices[i].NormalIndex].y;
		vertex.Normal.z = unstructuredMeshData.Normals[compactifiedMeshData.Vertices[i].NormalIndex].z;
		result.Vertices[i] = vertex;
	}

	result.Indices = new unsigned int[compactifiedMeshData.Indices.size()];
	for (unsigned int i = 0; i < compactifiedMeshData.Indices.size(); ++i)
	{
		result.Indices[i] = compactifiedMeshData.Indices[i];
	}

	return result;
}

UnstructuredMeshData ReadUnstructuredMeshDataFromFile(const char* filePath)
{
	UnstructuredMeshData result = {};

	std::ifstream inputFileStream(filePath, std::ios::in);
	if (!inputFileStream.is_open())
	{
		_RPTN(0, "Could not open OBJ file to load data\n", NULL);
	}

	char readLineInto[100];
	while (true)
	{
		inputFileStream.getline(readLineInto, 100, '\n');
		if (inputFileStream.eof())
		{
			break;
		}

		// copy line into separate string to use strtok on it
		char extractVerticesFrom[100];
		strcpy_s(extractVerticesFrom, readLineInto);
		char separators[] = " ";
		char* nextToken = nullptr;
		char* token = strtok_s(extractVerticesFrom, separators, &nextToken);

		// check if line contains vertex data
		vector<char*> tokens;
		switch (readLineInto[0])
		{
			case 'v': //vertex
			{
				// split line into tokens; first token is line identifier and is ignored
				while (token != NULL)
				{
					tokens.push_back(token);
					token = strtok_s(NULL, separators, &nextToken);
				}

				// convert tokens into vertex data
				switch (tokens[0][1])
				{
					case 'n':
						float3 normal;
						normal.x = strtof(tokens[1], nullptr);
						normal.y = strtof(tokens[2], nullptr);
						normal.z = strtof(tokens[3], nullptr);
						result.Normals.push_back(normal);
						break;

					case 't':
						float3 texel;
						texel.x = strtof(tokens[1], nullptr);
						texel.y = strtof(tokens[2], nullptr);
						if (tokens.size() > 3)
						{
							texel.z = strtof(tokens[3], nullptr);
						}
						else
						{
							texel.z = 0.0f;
						}
						result.Texels.push_back(texel);
						break;

					default:
						float3 position;
						position.x = strtof(tokens[1], nullptr);
						position.y = strtof(tokens[2], nullptr);
						position.z = strtof(tokens[3], nullptr);
						result.Positions.push_back(position);
						break;
				}
				break;
			}

			case 'f':
			{
				// split line into tokens
				while (token != NULL)
				{
					tokens.push_back(token);
					token = strtok_s(NULL, separators, &nextToken);
				}
				vector<UnstructuredVertex> tokenVertices;
				char faceSeparators[] = "/";

				// convert tokens into vertices
				for (unsigned int i = 1; i < tokens.size(); ++i)
				{
					char extractFacesFrom[100];
					strcpy_s(extractFacesFrom, tokens[i]);

					// split token into individual values
					vector<char*> faceTokens;
					token = strtok_s(extractFacesFrom, faceSeparators, &nextToken);
					while (token != NULL)
					{
						faceTokens.push_back(token);
						token = strtok_s(NULL, faceSeparators, &nextToken);
					}

					// convert values and add to list; raw values are 1-based, need to subtract 1 from each to make them 0-based
					UnstructuredVertex vertex;
					vertex.PositionIndex = strtoul(faceTokens[0], nullptr, 10) - 1;
					vertex.TexelIndex = strtoul(faceTokens[1], nullptr, 10) - 1;
					vertex.NormalIndex = strtoul(faceTokens[2], nullptr, 10) - 1;
					tokenVertices.push_back(vertex);
				}

				// assemble faces from vertex list, dividing quads into triangles if necessary
				OBJTriangle face = {};
				face.Vertices[0] = tokenVertices[0];
				face.Vertices[1] = tokenVertices[1];
				face.Vertices[2] = tokenVertices[2];
				result.Faces.push_back(face);
				if (tokens.size() > 4)
				{
					face.Vertices[0] = tokenVertices[2];
					face.Vertices[1] = tokenVertices[3];
					face.Vertices[2] = tokenVertices[0];
					result.Faces.push_back(face);
				}
				break;
			}

			default:
				break;
		}
	}

	inputFileStream.close();
	return result;
}


OBJMesh OBJMeshLoader::LoadOBJMesh(const char* filePath)
{
	UnstructuredMeshData unstructuredMeshData = ReadUnstructuredMeshDataFromFile(filePath);
	CompactifiedMeshData compactifiedMeshData = CompactifyUnstructuredMeshData(unstructuredMeshData);
	NormalizedMeshData normalizedMeshData = NormalizeStructuredMeshData(unstructuredMeshData, compactifiedMeshData);
	OBJMesh result =
	{
		normalizedMeshData.Vertices,
		compactifiedMeshData.Vertices.size(),
		normalizedMeshData.Indices,
		compactifiedMeshData.Indices.size(),
	};
	return result;
}

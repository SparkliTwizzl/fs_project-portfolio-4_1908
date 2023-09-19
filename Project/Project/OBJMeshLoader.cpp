#include "OBJMeshLoader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using std::vector;


const char OBJFaceIndicator = 'f';
const char OBJNormalIndicator = 'n';
const char OBJPositionIndicator = 'p';
const char OBJTexelIndicator = 't';
const char OBJVertexIndicator = 'v';


struct AbstractVertex
{
	unsigned int PositionIndex;
	unsigned int TexelIndex;
	unsigned int NormalIndex;
};

struct OBJTriangle
{
	AbstractVertex Vertices[3];
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
	vector<AbstractVertex> AbstractVertices;
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
			for (unsigned int unsortedVertexIndex = 0; unsortedVertexIndex < result.AbstractVertices.size(); ++unsortedVertexIndex)
			{

				if (unstructuredMeshData.Faces[faceIndex].Vertices[faceVertexIndex].PositionIndex == result.AbstractVertices[unsortedVertexIndex].PositionIndex
					&& unstructuredMeshData.Faces[faceIndex].Vertices[faceVertexIndex].TexelIndex == result.AbstractVertices[unsortedVertexIndex].TexelIndex
					&& unstructuredMeshData.Faces[faceIndex].Vertices[faceVertexIndex].NormalIndex == result.AbstractVertices[unsortedVertexIndex].NormalIndex)
				{
					isUnique = false;
					index = unsortedVertexIndex;
					break;
				}
			}

			if (isUnique)
			{
				index = result.AbstractVertices.size();
				result.AbstractVertices.push_back(unstructuredMeshData.Faces[faceIndex].Vertices[faceVertexIndex]);
			}

			result.Indices.push_back(index);
		}
	}

	return result;
}

NormalizedMeshData NormalizeStructuredMeshData(UnstructuredMeshData unstructuredMeshData, CompactifiedMeshData compactifiedMeshData)
{
	NormalizedMeshData result = {};

	result.Vertices = new OBJVertex[compactifiedMeshData.AbstractVertices.size()];
	for (unsigned int i = 0; i < compactifiedMeshData.AbstractVertices.size(); ++i)
	{
		OBJVertex vertex = {};
		vertex.Position.x = unstructuredMeshData.Positions[compactifiedMeshData.AbstractVertices[i].PositionIndex].x;
		vertex.Position.y = unstructuredMeshData.Positions[compactifiedMeshData.AbstractVertices[i].PositionIndex].y;
		vertex.Position.z = unstructuredMeshData.Positions[compactifiedMeshData.AbstractVertices[i].PositionIndex].z;
		vertex.Texel.x = unstructuredMeshData.Texels[compactifiedMeshData.AbstractVertices[i].TexelIndex].x;
		vertex.Texel.y = unstructuredMeshData.Texels[compactifiedMeshData.AbstractVertices[i].TexelIndex].y;
		vertex.Texel.z = unstructuredMeshData.Texels[compactifiedMeshData.AbstractVertices[i].TexelIndex].z;
		vertex.Normal.x = unstructuredMeshData.Normals[compactifiedMeshData.AbstractVertices[i].NormalIndex].x;
		vertex.Normal.y = unstructuredMeshData.Normals[compactifiedMeshData.AbstractVertices[i].NormalIndex].y;
		vertex.Normal.z = unstructuredMeshData.Normals[compactifiedMeshData.AbstractVertices[i].NormalIndex].z;
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

		char extractVerticesFrom[100];
		strcpy_s(extractVerticesFrom, readLineInto);

		char separators[] = " ";
		char* nextToken = nullptr;
		char* token = strtok_s(extractVerticesFrom, separators, &nextToken);

		vector<char*> tokens;
		while (token != NULL)
		{
			tokens.push_back(token);
			token = strtok_s(NULL, separators, &nextToken);
		}

		char objLineTypeIndicator = readLineInto[0];
		switch (objLineTypeIndicator)
		{
			case OBJVertexIndicator:
			{
				// convert tokens into vertex data; first token is line identifier and is ignored
				switch (tokens[0][1])
				{
					case OBJNormalIndicator:
					{
						float3 normal =
						{
							.x = strtof(tokens[1], nullptr),
							.y = strtof(tokens[2], nullptr),
							.z = strtof(tokens[3], nullptr),
						};
						result.Normals.push_back(normal);
						break;
					}

					case OBJTexelIndicator:
					{
						bool doesTexelHaveZValue = tokens.size() > 3;
						float3 texel =
						{
							.x = strtof(tokens[1], nullptr),
							.y = strtof(tokens[2], nullptr),
							.z = (doesTexelHaveZValue ? strtof(tokens[3], nullptr) : 0.0f),
						};
						result.Texels.push_back(texel);
						break;
					}

					case OBJPositionIndicator:
					default:
					{
						float3 position =
						{
							.x = strtof(tokens[1], nullptr),
							.y = strtof(tokens[2], nullptr),
							.z = strtof(tokens[3], nullptr),
						};
						result.Positions.push_back(position);
						break;
					}
				}
				break;
			}

			case OBJFaceIndicator:
			{
				// convert tokens into vertices
				vector<AbstractVertex> tokenVertices;
				char faceSeparators[] = "/";
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
					AbstractVertex vertex;
					vertex.PositionIndex = strtoul(faceTokens[0], nullptr, 10) - 1;
					vertex.TexelIndex = strtoul(faceTokens[1], nullptr, 10) - 1;
					vertex.NormalIndex = strtoul(faceTokens[2], nullptr, 10) - 1;
					tokenVertices.push_back(vertex);
				}

				OBJTriangle face =
				{
					tokenVertices[0],
					tokenVertices[1],
					tokenVertices[2],
				};
				result.Faces.push_back(face);

				bool isFaceQuad = tokens.size() > 4;
				if (isFaceQuad)
				{
					face =
					{
						tokenVertices[2],
						tokenVertices[3],
						tokenVertices[0],
					};
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
		compactifiedMeshData.AbstractVertices.size(),
		normalizedMeshData.Indices,
		compactifiedMeshData.Indices.size(),
	};
	return result;
}

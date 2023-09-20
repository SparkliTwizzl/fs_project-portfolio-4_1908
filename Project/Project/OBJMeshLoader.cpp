#include "OBJMeshLoader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using std::string;
using std::vector;


const char OBJFaceIndicator = 'f';
const char OBJNormalIndicator = 'n';
const char OBJPositionIndicator = 'p';
const char OBJTexelIndicator = 't';
const char OBJVertexIndicator = 'v';


struct AbstractVertex
{
	unsigned int NormalIndex;
	unsigned int PositionIndex;
	unsigned int TexelIndex;

	bool operator==(const AbstractVertex& other)
	{
		bool areEqual = NormalIndex == other.NormalIndex
			&& PositionIndex == other.PositionIndex
			&& TexelIndex == other.TexelIndex;
		return areEqual;
	}
};

struct OBJTriangle
{
	AbstractVertex Vertices[3];
};

struct UnstructuredMeshData
{
	vector<OBJTriangle> Faces;
	vector<float3> Normals;
	vector<float3> Positions;
	vector<float3> Texels;
};

struct CompactifiedMeshData
{
	vector<unsigned int> Indices;
	vector<AbstractVertex> Vertices;
};

struct NormalizedMeshData
{
	unsigned int* Indices;
	OBJVertex* Vertices;
};


CompactifiedMeshData CompactifyUnstructuredMeshData(UnstructuredMeshData unstructuredMeshData)
{
	CompactifiedMeshData result = {};
	for (unsigned int faceIndex = 0; faceIndex < unstructuredMeshData.Faces.size(); ++faceIndex)
	{
		for (unsigned int faceVertexIndex = 0; faceVertexIndex < 3; ++faceVertexIndex)
		{
			bool isVertexUnique = true;
			unsigned int indexOfVertex = 0;
			for (unsigned int unsortedVertexIndex = 0; unsortedVertexIndex < result.Vertices.size(); ++unsortedVertexIndex)
			{
				AbstractVertex faceVertex = unstructuredMeshData.Faces[faceIndex].Vertices[faceVertexIndex];
				AbstractVertex unsortedVertex = result.Vertices[unsortedVertexIndex];
				if (faceVertex == unsortedVertex)
				{
					isVertexUnique = false;
					indexOfVertex = unsortedVertexIndex;
					break;
				}
			}

			if (isVertexUnique)
			{
				indexOfVertex = result.Vertices.size();
				result.Vertices.push_back(unstructuredMeshData.Faces[faceIndex].Vertices[faceVertexIndex]);
			}

			result.Indices.push_back(indexOfVertex);
		}
	}

	return result;
}

NormalizedMeshData NormalizeStructuredMeshData(UnstructuredMeshData unstructuredMeshData, CompactifiedMeshData compactifiedMeshData)
{
	NormalizedMeshData result = {};

	result.Indices = new unsigned int[compactifiedMeshData.Indices.size()];
	std::copy(compactifiedMeshData.Indices.begin(), compactifiedMeshData.Indices.end(), result.Indices);

	vector<OBJVertex> objVertices;
	for (unsigned int i = 0; i < compactifiedMeshData.Vertices.size(); ++i)
	{
		AbstractVertex abstractVertex = compactifiedMeshData.Vertices[i];
		OBJVertex objVertex =
		{
			.Normal = unstructuredMeshData.Normals[abstractVertex.NormalIndex],
			.Position = unstructuredMeshData.Positions[abstractVertex.PositionIndex],
			.Texel = unstructuredMeshData.Texels[abstractVertex.TexelIndex],
		};
		objVertices.push_back(objVertex);
	}
	result.Vertices = new OBJVertex[compactifiedMeshData.Vertices.size()];
	std::copy(objVertices.begin(), objVertices.end(), result.Vertices);

	return result;
}

UnstructuredMeshData ReadUnstructuredMeshDataFromFile(string filePath)
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


OBJMesh OBJMeshLoader::LoadOBJMesh(string filePath)
{
	UnstructuredMeshData unstructuredMeshData = ReadUnstructuredMeshDataFromFile(filePath);
	CompactifiedMeshData compactifiedMeshData = CompactifyUnstructuredMeshData(unstructuredMeshData);
	NormalizedMeshData normalizedMeshData = NormalizeStructuredMeshData(unstructuredMeshData, compactifiedMeshData);
	OBJMesh result =
	{
		normalizedMeshData.Indices,
		compactifiedMeshData.Indices.size(),
		normalizedMeshData.Vertices,
		compactifiedMeshData.Vertices.size(),
	};
	return result;
}

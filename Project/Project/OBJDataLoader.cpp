#include "OBJDataLoader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using std::vector;


struct OBJTriangle
{
	uint3 VertexIndices[3];
};

struct RawMeshData
{
	vector<float3> Positions;
	vector<float3> Texels;
	vector<float3> Normals;
	vector<OBJTriangle> Faces;
};





OBJMesh OBJMeshLoader::LoadOBJMesh(const char* filePath)
{
	RawMeshData rawMeshData = {};
	std::ifstream inputFileStream(filePath, std::ios::in);
	char readLineInto[100];

	// ----- PROCESS RAW DATA INTO LISTS -----

	// verify that file opened
	if (!inputFileStream.is_open())
	{
		_RPTN(0, "Could not open OBJ file to load data\n", NULL);
	}
	// read data from file
	while (true)
	{
		// read next line
		inputFileStream.getline(readLineInto, 100, '\n');
		if (inputFileStream.eof())
		{
			break;
		}
		// copy line into separate string for strtok
		char extractVerticesFrom[100];
		strcpy_s(extractVerticesFrom, readLineInto);
		char separators[] = " ";
		char *token, *nextToken;
		std::vector<char*> tokens;
		token = strtok_s(extractVerticesFrom, separators, &nextToken);
		// check if line contains vertex data
		switch (readLineInto[0])
		{
			case 'v': //vertex
			{
				// split line into tokens
				// first token is line identifier and is ignored
				while (token != NULL)
				{
					tokens.push_back(token);
					token = strtok_s(NULL, separators, &nextToken);
				}

				// convert tokens into vertex data
				switch (tokens[0][1])
				{
					case 'n': // normal
						float3 normal;
						normal.x = strtof(tokens[1], nullptr);
						normal.y = strtof(tokens[2], nullptr);
						normal.z = strtof(tokens[3], nullptr);
						rawMeshData.Normals.push_back(normal);
						break;
					case 't': // texel
						float3 texel;
						texel.x = strtof(tokens[1], nullptr);
						texel.y = strtof(tokens[2], nullptr);
						if (tokens.size() > 3)
							texel.z = strtof(tokens[3], nullptr);
						else
							texel.z = 0.0f;
						rawMeshData.Texels.push_back(texel);
						break;
					default: // position
						float3 position;
						position.x = strtof(tokens[1], nullptr);
						position.y = strtof(tokens[2], nullptr);
						position.z = strtof(tokens[3], nullptr);
						rawMeshData.Positions.push_back(position);
						break;
				}
				break;
			}
			case 'f': // face
			{
				// split line into tokens
				while (token != NULL)
				{
					tokens.push_back(token);
					token = strtok_s(NULL, separators, &nextToken);
				}
				std::vector<uint3> tokenVertices;
				char faceSeparators[] = "/";

				// convert tokens into vertices
				for (unsigned int i = 1; i < tokens.size(); ++i)
				{
					// copy token to split further
					char extractFacesFrom[100];
					strcpy_s(extractFacesFrom, tokens[i]);

					// split token into individual values
					std::vector<char*> faceTokens;
					token = strtok_s(extractFacesFrom, faceSeparators, &nextToken);
					while (token != NULL)
					{
						faceTokens.push_back(token);
						token = strtok_s(NULL, faceSeparators, &nextToken);
					}

					// store values and add to list
					// raw values are 1-based, need to subtract 1 from each to make them 0-based
					uint3 vertex;
					vertex.x = strtoul(faceTokens[0], nullptr, 10) - 1;
					vertex.y = strtoul(faceTokens[1], nullptr, 10) - 1;
					vertex.z = strtoul(faceTokens[2], nullptr, 10) - 1;
					tokenVertices.push_back(vertex);
				}

				// assemble faces from vertex list, dividing quads into triangles if necessary
				OBJTriangle face = {};
				face.VertexIndices[0] = tokenVertices[0];
				face.VertexIndices[1] = tokenVertices[1];
				face.VertexIndices[2] = tokenVertices[2];
				rawMeshData.Faces.push_back(face);
				if (tokens.size() > 4)
				{
					face.VertexIndices[0] = tokenVertices[2];
					face.VertexIndices[1] = tokenVertices[3];
					face.VertexIndices[2] = tokenVertices[0];
					rawMeshData.Faces.push_back(face);
				}
				break;
			}
			default:
				break;
		}
	}
	inputFileStream.close();
	// ----- PROCESS RAW DATA INTO LISTS -----

	// ----- CONVERT LISTS INTO VERT / IND DATA -----
	std::vector<uint3> rawVertexData;
	std::vector<unsigned int> indexList;
	// iterate through faces
	for (unsigned int faceIndex = 0; faceIndex < rawMeshData.Faces.size(); faceIndex++)
	{
		// iterate through face's vertices
		for (unsigned int faceVertexIndex = 0; faceVertexIndex < 3; faceVertexIndex++)
		{
			bool isUnique = true;
			unsigned int index = 0;
			// iterate through vertex list
			for (unsigned int vertexListIndex = 0; vertexListIndex < rawVertexData.size(); vertexListIndex++)
			{
				// check vert values against current list item values
				if (rawMeshData.Faces[faceIndex].VertexIndices[faceVertexIndex].x == rawVertexData[vertexListIndex].x
					&& rawMeshData.Faces[faceIndex].VertexIndices[faceVertexIndex].y == rawVertexData[vertexListIndex].y
					&& rawMeshData.Faces[faceIndex].VertexIndices[faceVertexIndex].z == rawVertexData[vertexListIndex].z)
				{
					// vertex is duplicate, store index where it was found
					isUnique = false;
					index = vertexListIndex;
					break;
				}
			}
			// if vert is unique, add to list and store index where it was added
			if (isUnique)
			{
				index = rawVertexData.size();
				rawVertexData.push_back(rawMeshData.Faces[faceIndex].VertexIndices[faceVertexIndex]);
			}
			// add index of vert to list
			indexList.push_back(index);
		}
	}
	// ----- CONVERT LISTS INTO VERT / IND DATA -----

	// ----- CONVERT VERT DATA INTO VERTS AND COPY INTO ARRAY -----
	OBJVertex* vertices = new OBJVertex[rawVertexData.size()];
	for (unsigned int i = 0; i < rawVertexData.size(); ++i)
	{
		OBJVertex vertex = {};
		vertex.Position.x = rawMeshData.Positions[rawVertexData[i].x].x;
		vertex.Position.y = rawMeshData.Positions[rawVertexData[i].x].y;
		vertex.Position.z = rawMeshData.Positions[rawVertexData[i].x].z;
		vertex.Texel.x = rawMeshData.Texels[rawVertexData[i].y].x;
		vertex.Texel.y = rawMeshData.Texels[rawVertexData[i].y].y;
		vertex.Texel.z = rawMeshData.Texels[rawVertexData[i].y].z;
		vertex.Normal.x = rawMeshData.Normals[rawVertexData[i].z].x;
		vertex.Normal.y = rawMeshData.Normals[rawVertexData[i].z].y;
		vertex.Normal.z = rawMeshData.Normals[rawVertexData[i].z].z;
		vertices[i] = vertex;
	}
	// ----- CONVERT VERT DATA INTO VERTS AND COPY INTO ARRAY -----

	// ----- COPY INDEX LIST INTO ARRAY -----
	unsigned int* indices = new unsigned int[indexList.size()];
	for (unsigned int i = 0; i < indexList.size(); ++i)
	{
		indices[i] = indexList[i];
	}
	// ----- COPY INDEX LIST INTO ARRAY -----

	OBJMesh mesh =
	{
		vertices,
		rawVertexData.size(),
		indices,
		indexList.size(),
	};
	return mesh;
}

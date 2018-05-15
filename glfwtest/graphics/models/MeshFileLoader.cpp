#include "MeshFileLoader.h"

#include "io/FileSystem.h"
#include "graphics/models/Mesh.h"
#include "graphics/models/SubMesh.h"
#include "graphics/helpers/color.h"

#define TINYGLTF_IMPLEMENTATION
//#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE

#include "tinygltf/tiny_gltf.h"
#include "tinyobj/tiny_obj_loader.h"
#include "graphics/models/importers/stl_loader.h"

#include "physics/aabb.h"

#include <unordered_map>

namespace std 
{
	template<> struct hash<VertexPTCN> 
	{
		size_t operator()(VertexPTCN const& vertex) const 
		{
			return ((hash<glm::vec3>()(vertex.pos) ^
					(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
					(hash<glm::vec2>()(vertex.texCoords) << 1) ^
					(hash<glm::vec3>()(vertex.normal) << 1);
		}
	};
}

MeshFileLoader::MeshFileLoader()
{

}

MeshFileLoader::~MeshFileLoader()
{

}

//TODO: implement glTF? https://godotengine.org/article/we-should-all-use-gltf-20-export-3d-assets-game-engines
std::shared_ptr<Mesh> MeshFileLoader::Get(const std::string& fileName)
{
	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

	std::string extension = Helpers::GetFileExtension(fileName);
	
	MeshFileType::Enum fileType;

	if (extension == "obj")
		fileType = MeshFileType::OBJ;
	else if (extension == "stl")
		fileType = MeshFileType::STL;
	else if (extension == "gltf" || extension == "glb")
		fileType = MeshFileType::GLTF;
	else
	{
		std::cout << "Unhandled file extension: " << extension << std::endl;
		assert(false);
	}

	FileSystem::LoadFileAsync("models/" + fileName, std::bind(MeshFileLoader::FileLoaded, std::placeholders::_1, mesh, fileType));

	return mesh;
}

void MeshFileLoader::FileLoaded(std::vector<char> fileData, std::shared_ptr<Mesh> meshDestination, MeshFileType::Enum fileType)
{
	switch (fileType)
	{
	case MeshFileType::OBJ:
		LoadOBJ(fileData, meshDestination);
		break;
	case MeshFileType::STL:
		LoadSTL(fileData, meshDestination);
		break;
	case MeshFileType::GLTF:
		LoadGLTF(fileData, meshDestination);
		break;
	default:
		break;
	}
}

void MeshFileLoader::LoadGLTF(std::vector<char>& fileData, std::shared_ptr<Mesh> meshDestination)
{
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;

	bool ret = loader.LoadBinaryFromMemory(&model, &err, reinterpret_cast<unsigned char*>(&fileData[0]), fileData.size());
	//bool ret = loader.LoadBinaryFromFile(&model, &err, argv[1]); // for binary glTF(.glb) 
	if (!err.empty()) 
	{
		printf("Err: %s\n", err.c_str());
	}

	if (!ret) 
	{
		printf("Failed to parse glTF\n");
	}

	for (const tinygltf::BufferView& bufferView : model.bufferViews)
	{
		if (bufferView.target == 0)
		{
			std::cout << "WARN: bufferView.target == 0" << std::endl;
			continue;
		}

		const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
		std::cout << "buffer.size= " << buffer.data.size() << ", byteOffset = " << bufferView.byteOffset << std::endl;
	}

	auto meshes = model.meshes;
	for (tinygltf::Mesh& mesh : meshes)
	{
		std::vector<VertexPTCN> vertices;
		std::vector<uint32_t> indices;

		for (const auto& primitive : mesh.primitives)
		{
			if (primitive.indices < 0)
			{
				std::cout << "WARN: primitive.indices < 0" << std::endl;
				continue;
			}

			for (const auto& attribute : primitive.attributes)
			{
				const tinygltf::Accessor& accessor = model.accessors[attribute.second];

				int size = 1;
				if (accessor.type == TINYGLTF_TYPE_SCALAR) 
				{
					size = 1;
				}
				else if (accessor.type == TINYGLTF_TYPE_VEC2) 
				{
					size = 2;
				}
				else if (accessor.type == TINYGLTF_TYPE_VEC3) 
				{
					size = 3;
				}
				else if (accessor.type == TINYGLTF_TYPE_VEC4) 
				{
					size = 4;
				}
				else 
				{
					assert(false && "not supported");
				}

				if ((attribute.first.compare("POSITION") == 0) ||
					(attribute.first.compare("NORMAL") == 0) ||
					(attribute.first.compare("TEXCOORD_0") == 0)) 
				{
					int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);

					int asd = 0;
				}
			}

			/*VertexPTCN vertex = {};

			vertex.pos =
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			aabb.Grow(vertex.pos);

			vertex.texCoords =
			{
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = colors[colorIndex];

			vertex.normal =
			{
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};

			vertex.normal = glm::normalize(vertex.normal);

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}
			else
			{
				skippedVertices++;
			}

			indices.push_back(uniqueVertices[vertex]);*/
		}
	}
}


void MeshFileLoader::LoadOBJ(std::vector<char>& fileData, std::shared_ptr<Mesh> meshDestination)
{
	VectorBuffer<char> dataBuffer(fileData);
	std::istream is(&dataBuffer);


	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &is))
	{
		throw std::runtime_error(err);
	}

	int colorIndex = 0;
	//std::array<Color, 20> colors = { Color::Red, Color::Green, Color::Yellow, Color::Blue, Color::Orange, Color::Purple, Color::Cyan, Color::Magenta, Color::Lime, Color::Pink, Color::Teal, Color::Lavender, Color::Brown, Color::Beige, Color::Maroon, Color::Mint, Color::Olive, Color::Coral, Color::Navy, Color::Grey };
	std::array<Color, 1> colors = { Color::White };

	for (const auto& shape : shapes)
	{
		std::vector<VertexPTCN> vertices;
		std::vector<uint32_t> indices;

		std::unordered_map<VertexPTCN, uint32_t> uniqueVertices = {};
		uint32_t skippedVertices = 0;

		AABB aabb;

		for (const auto& index : shape.mesh.indices)
		{
			VertexPTCN vertex = {};

			vertex.pos =
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			aabb.Grow(vertex.pos);

			vertex.texCoords =
			{
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = colors[colorIndex];

			vertex.normal = 
			{
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};

			vertex.normal = glm::normalize(vertex.normal);

			if (uniqueVertices.count(vertex) == 0) 
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}
			else
			{
				skippedVertices++;
			}

			indices.push_back(uniqueVertices[vertex]);
		}

		colorIndex = (colorIndex + 1) % colors.size();
		std::cout << "Skipped " << skippedVertices << " vertices." << std::endl;

		uint32_t memorySize = sizeof(VertexPTCN) * vertices.size();
		uint8_t* vertexData = new uint8_t[memorySize];
		memcpy(vertexData, vertices.data(), memorySize);

		uint32_t memorySizeIndices = sizeof(uint32_t) * indices.size();
		uint8_t* indexData = new uint8_t[memorySizeIndices];
		memcpy(indexData, indices.data(), memorySizeIndices);

		uint32_t triangleCount = static_cast<uint32_t>(indices.size()) / 3;
		SubMesh* subMesh = meshDestination->AllocateBuffers(vertexData, memorySize, indexData, memorySizeIndices, triangleCount);
		subMesh->SetAABB(aabb);

		delete vertexData;
		delete indexData;
	}
	
	/*for (size_t i = 0; i < meshDestination->triangleCount; i++)
	{
		VertexPTCN v1 = vertices[3 * i + 0];
		VertexPTCN v2 = vertices[3 * i + 1];
		VertexPTCN v3 = vertices[3 * i + 2];

		glm::vec3 edge1 = v1.pos - v2.pos;
		glm::vec3 edge2 = v1.pos - v3.pos;
		glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

		//Normal to color
		vertices[3 * i + 0].normal = normal;
		vertices[3 * i + 1].normal = normal;
		vertices[3 * i + 2].normal = normal;
	}*/

	VertexPTCN::GetBindingDescription(meshDestination->bindingDescription);
	VertexPTCN::GetAttributeDescriptions(meshDestination->attributeDescriptions);

}

void MeshFileLoader::LoadSTL(const std::vector<char>& fileData, std::shared_ptr<Mesh> meshDestination)
{
	STLModel modelData;
	STL::ReadBinary(fileData, modelData);

	std::vector<Vertex3D> vertices;
	std::vector<uint32_t> indices;

	for (size_t i = 0; i < modelData.triangleCount; i++)
	{
		const STLTriangle& triangle = modelData.triangles[i];

		Vertex3D vertexA = {};
		Vertex3D vertexB = {};
		Vertex3D vertexC = {};

		vertexA.pos = triangle.position[0];
		vertexB.pos = triangle.position[1];
		vertexC.pos = triangle.position[2];

		vertexA.texCoords = glm::vec2(0, 0);
		vertexB.texCoords = glm::vec2(0, 0);
		vertexC.texCoords = glm::vec2(0, 0);
		
		vertexA.color = glm::vec3(0.5f, 0.5f, 0.5f) + (triangle.normal * 0.5f);
		vertexB.color = glm::vec3(0.5f, 0.5f, 0.5f) + (triangle.normal * 0.5f);
		vertexC.color = glm::vec3(0.5f, 0.5f, 0.5f) + (triangle.normal * 0.5f);

		vertices.push_back(vertexA);
		vertices.push_back(vertexB);
		vertices.push_back(vertexC);

		indices.push_back(static_cast<uint32_t>(indices.size()));
		indices.push_back(static_cast<uint32_t>(indices.size()));
		indices.push_back(static_cast<uint32_t>(indices.size()));
	}


	Vertex3D::GetBindingDescription(meshDestination->bindingDescription);
	Vertex3D::GetAttributeDescriptions(meshDestination->attributeDescriptions);

	uint32_t memorySize = sizeof(Vertex3D) * vertices.size();
	uint8_t* vertexData = new uint8_t[memorySize];
	memcpy(vertexData, vertices.data(), memorySize);

	uint32_t memorySizeIndices = sizeof(uint32_t) * indices.size();
	uint8_t* indexData = new uint8_t[memorySizeIndices];
	memcpy(indexData, indices.data(), memorySizeIndices);

	uint32_t triangleCount = static_cast<uint32_t>(indices.size()) / 3;
	meshDestination->AllocateBuffers(vertexData, memorySize, indexData, memorySizeIndices, triangleCount);

	delete vertexData;
	delete indexData;
}
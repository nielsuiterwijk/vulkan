#include "MeshFileLoader.h"

#include "io/FileSystem.h"
#include "graphics/models/Mesh.h"

#include "tinyobj/tiny_obj_loader.h"
#include "graphics/models/importers/stl_loader.h"

MeshFileLoader::MeshFileLoader()
{

}

MeshFileLoader::~MeshFileLoader()
{

}

//TODO: implement glTF? https://godotengine.org/article/we-should-all-use-gltf-20-export-3d-assets-game-engines
//TODO: Implement "MeshFileLoader" so it can load glTF & STL files
std::shared_ptr<Mesh> MeshFileLoader::Get(const std::string& fileName)
{
	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

	std::string extension = Helpers::GetFileExtension(fileName);
	
	MeshFileType::Enum fileType;

	if (extension == "obj")
		fileType = MeshFileType::OBJ;
	else if (extension == "stl")
		fileType = MeshFileType::STL;
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
	case MeshFileLoader::MeshFileType::OBJ:
		LoadOBJ(fileData, meshDestination);
		break;
	case MeshFileLoader::MeshFileType::STL:
		LoadSTL(fileData, meshDestination);
		break;
	default:
		break;
	}
}


void MeshFileLoader::LoadOBJ(std::vector<char>& fileData, std::shared_ptr<Mesh> meshDestination)
{
	VectorBuffer<char> dataBuffer(fileData);
	std::istream is(&dataBuffer);

	std::vector<VertexPTCN> vertices;
	std::vector<uint32_t> indices;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &is))
	{
		throw std::runtime_error(err);
	}

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			VertexPTCN vertex = {};

			vertex.pos =
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoords =
			{
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color =
			{
				1.0f, 1.0f, 1.0f
			};

			vertex.normal = 
			{
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};

			vertex.normal = glm::normalize(vertex.normal);

			vertices.push_back(vertex);
			indices.push_back(static_cast<uint32_t>(indices.size()));
		}
	}

	meshDestination->triangleCount = static_cast<uint32_t>(indices.size()) / 3;

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

	uint32_t memorySize = sizeof(VertexPTCN) * vertices.size();
	uint8_t* vertexData = new uint8_t[memorySize];
	memcpy(vertexData, vertices.data(), memorySize);

	uint32_t memorySizeIndices = sizeof(uint32_t) * indices.size();
	uint8_t* indexData = new uint8_t[memorySizeIndices];
	memcpy(indexData, indices.data(), memorySizeIndices);

	meshDestination->AllocateBuffers(vertexData, memorySize, indexData, memorySizeIndices);

	delete vertexData;
	delete indexData;
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

	meshDestination->triangleCount = static_cast<uint32_t>(indices.size()) / 3;

	Vertex3D::GetBindingDescription(meshDestination->bindingDescription);
	Vertex3D::GetAttributeDescriptions(meshDestination->attributeDescriptions);

	uint32_t memorySize = sizeof(Vertex3D) * vertices.size();
	uint8_t* vertexData = new uint8_t[memorySize];
	memcpy(vertexData, vertices.data(), memorySize);

	uint32_t memorySizeIndices = sizeof(uint32_t) * indices.size();
	uint8_t* indexData = new uint8_t[memorySizeIndices];
	memcpy(indexData, indices.data(), memorySizeIndices);

	meshDestination->AllocateBuffers(vertexData, memorySize, indexData, memorySizeIndices);

	delete vertexData;
	delete indexData;
}
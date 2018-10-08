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
	template<> struct hash<Vertex> 
	{
		size_t operator()(Vertex const& vertex) const 
		{
			return ((hash<glm::vec3>()(vertex.pos) ^
					(hash<glm::vec4>()(vertex.color) << 1)) >> 1) ^
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
	std::string warning;

	bool ret = loader.LoadBinaryFromMemory(&model, &err, &warning, reinterpret_cast<unsigned char*>(&fileData[0]), fileData.size());
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
		std::cout << "buffer.size= " << buffer.data.size() << ", byteOffset= " << bufferView.byteOffset << std::endl;
	}

	auto meshes = model.meshes;
	for (tinygltf::Mesh& mesh : meshes)
	{		
		for (const auto& primitive : mesh.primitives)
		{
			if (primitive.indices < 0)
			{
				std::cout << "WARN: primitive.indices < 0" << std::endl;
				continue;
			}

			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;

			AABB aabb;

			const float* bufferPos = nullptr;
			const float* bufferNormals = nullptr;
			const float* bufferTexCoords = nullptr;
			const uint16_t* bufferJoints = nullptr;
			const float* bufferWeights = nullptr;

			assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

			const tinygltf::Accessor &posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
			const tinygltf::BufferView &posView = model.bufferViews[posAccessor.bufferView];
			bufferPos = reinterpret_cast<const float *>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));

			if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) 
			{
				const tinygltf::Accessor &normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
				const tinygltf::BufferView &normView = model.bufferViews[normAccessor.bufferView];
				bufferNormals = reinterpret_cast<const float *>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
			}

			if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
				const tinygltf::Accessor &uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
				const tinygltf::BufferView &uvView = model.bufferViews[uvAccessor.bufferView];
				bufferTexCoords = reinterpret_cast<const float *>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
			}

			// Skinning
			// Joints
			if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end()) 
			{
				const tinygltf::Accessor &jointAccessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
				const tinygltf::BufferView &jointView = model.bufferViews[jointAccessor.bufferView];
				bufferJoints = reinterpret_cast<const uint16_t *>(&(model.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]));
			}

			if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end())
			{
				const tinygltf::Accessor &uvAccessor = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
				const tinygltf::BufferView &uvView = model.bufferViews[uvAccessor.bufferView];
				bufferWeights = reinterpret_cast<const float *>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
			}

			bool hasSkin = (bufferJoints && bufferWeights);

			for (size_t v = 0; v < posAccessor.count; v++) 
			{
				Vertex vertex {};
				vertex.pos = glm::make_vec3(&bufferPos[v * 3]);
				vertex.normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * 3]) : glm::vec3(0.0f)));
				vertex.texCoords = bufferTexCoords ? glm::make_vec2(&bufferTexCoords[v * 2]) : glm::vec3(0.0f);
				vertex.color = glm::vec4(1.0f);

				//vert.joint0 = hasSkin ? glm::vec4(glm::make_vec4(&bufferJoints[v * 4])) : glm::vec4(0.0f);
				//vert.weight0 = hasSkin ? glm::make_vec4(&bufferWeights[v * 4]) : glm::vec4(0.0f);
				vertices.emplace_back(vertex);

				aabb.Grow(vertex.pos);
			}



			// Indices
			{
				const tinygltf::Accessor &accessor = model.accessors[primitive.indices];
				const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

				uint32_t indexCount = static_cast<uint32_t>(accessor.count);

				switch (accessor.componentType) {
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
					const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
					for (size_t index = 0; index < accessor.count; index++) {
						indices.push_back(buf[index]);
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
					const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);

					for (size_t index = 0; index < accessor.count; index++) {
						indices.push_back(buf[index]);
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
					const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
					for (size_t index = 0; index < accessor.count; index++) {
						indices.push_back(buf[index]);
					}
					break;
				}
				default:
					std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
					return;
				}
			}

			
			uint32_t triangleCount = static_cast<uint32_t>(indices.size()) / 3;
			SubMesh* subMesh = meshDestination->AllocateBuffers(static_cast<void*>(&vertices[0]), sizeof(Vertex) * vertices.size(),
																static_cast<void*>(&indices[0]), sizeof(uint32_t) * indices.size(), triangleCount);
			subMesh->SetAABB(aabb);
		}


		//VertexPTCN::GetBindingDescription(meshDestination->bindingDescription);
		//VertexPTCN::GetAttributeDescriptions(meshDestination->attributeDescriptions);
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
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		std::unordered_map<Vertex, uint32_t> uniqueVertices = {};
		uint32_t skippedVertices = 0;

		AABB aabb;

		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex = {};

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

		uint32_t memorySize = sizeof(Vertex) * vertices.size();
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
	
	//VertexPTCN::GetBindingDescription(meshDestination->bindingDescription);
	//VertexPTCN::GetAttributeDescriptions(meshDestination->attributeDescriptions);

}

void MeshFileLoader::LoadSTL(const std::vector<char>& fileData, std::shared_ptr<Mesh> meshDestination)
{
	STLModel modelData;
	STL::ReadBinary(fileData, modelData);

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (size_t i = 0; i < modelData.triangleCount; i++)
	{
		const STLTriangle& triangle = modelData.triangles[i];

		Vertex vertexA = {};
		Vertex vertexB = {};
		Vertex vertexC = {};

		vertexA.pos = triangle.position[0];
		vertexB.pos = triangle.position[1];
		vertexC.pos = triangle.position[2];

		vertexA.texCoords = glm::vec2(0, 0);
		vertexB.texCoords = glm::vec2(0, 0);
		vertexC.texCoords = glm::vec2(0, 0);
		
		vertexA.color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) + glm::vec4(triangle.normal * 0.5f, 1.0f);
		vertexB.color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) + glm::vec4(triangle.normal * 0.5f, 1.0f);
		vertexC.color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) + glm::vec4(triangle.normal * 0.5f, 1.0f);

		vertices.push_back(vertexA);
		vertices.push_back(vertexB);
		vertices.push_back(vertexC);

		indices.push_back(static_cast<uint32_t>(indices.size()));
		indices.push_back(static_cast<uint32_t>(indices.size()));
		indices.push_back(static_cast<uint32_t>(indices.size()));
	}


	//Vertex3D::GetBindingDescription(meshDestination->bindingDescription);
	//Vertex3D::GetAttributeDescriptions(meshDestination->attributeDescriptions);

	uint32_t memorySize = sizeof(Vertex) * vertices.size();
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
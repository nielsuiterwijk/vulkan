#include "MeshFileLoader.h"

#include "io/FileSystem.h"
#include "graphics/models/Mesh.h"
#include "graphics/models/SkinnedMesh.h"
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

std::shared_ptr<Mesh> MeshFileLoader::Static(const std::string& fileName)
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

std::shared_ptr<Mesh> MeshFileLoader::Skinned(const std::string& fileName)
{
	std::shared_ptr<Mesh> mesh = std::make_shared<SkinnedMesh>();

	std::string extension = Helpers::GetFileExtension(fileName);

	MeshFileType::Enum fileType;

	if (extension == "gltf" || extension == "glb")
	{
		fileType = MeshFileType::GLTF;
	}
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

namespace
{
	void TraverseBones(int32_t targetBone, const std::vector<BoneInfo>& bones, std::vector<int32_t>& boneOrder)
	{
		boneOrder.emplace_back(targetBone);
		const BoneInfo& boneInfo = bones[targetBone];

		for (int32_t child : boneInfo.children)
		{
			TraverseBones(child, bones, boneOrder);
		}
	}
}

void MeshFileLoader::LoadNode(BoneInfo* parent, uint32_t nodeIndex, const tinygltf::Model& model, std::shared_ptr<SkinnedMesh> skinnedMesh)
{
	if (parent != nullptr)
	{
		assert(std::find(parent->children.begin(), parent->children.end(), nodeIndex) != parent->children.end());
	}

	const tinygltf::Node& node = model.nodes[nodeIndex];

	glm::vec3 position = node.translation.size() == 0 ? glm::vec3(0.0f) : glm::make_vec3(node.translation.data());
	glm::vec3 scale = node.scale.size() == 0 ? glm::vec3(1.0f) : glm::make_vec3(node.scale.data());
	glm::quat rotation = node.rotation.size() == 0 ? glm::quat() : glm::make_quat(node.rotation.data());

	glm::mat4 transform = node.matrix.size() == 0 ? glm::mat4() : glm::make_mat4x4(node.matrix.data());

	BoneInfo& bone = skinnedMesh->bones[nodeIndex];
	bone.index = nodeIndex;
	bone.children = node.children;

	bone.scale = scale;// glm::scale(glm::mat4(1.0f), scale);
	bone.rotation = rotation;// glm::mat4(rotation);
	bone.translation = position;// glm::translate(glm::mat4(1.0f), position);

	if (parent != nullptr)
	{
		bone.parent = parent->index;
	}

	//bone.localTransform = glm::translate(position) * glm::toMat4(rotation) * glm::scale(scale);
	bone.localTransform = transform;

	for (int32_t childIndex : bone.children) 
	{
		LoadNode(&skinnedMesh->bones[nodeIndex], childIndex, model, skinnedMesh);
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
		meshDestination = nullptr;
		return;
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
	
	GLTFStaticMesh(&model, meshDestination);

	if (meshDestination->GetMeshType() == MeshType::Skinned)
	{
		std::shared_ptr<SkinnedMesh> skinnedMesh = std::static_pointer_cast<SkinnedMesh>(meshDestination);
		skinnedMesh->bones.reserve(model.nodes.size());

		tinygltf::Node rootNode;
		if (model.skins.size() > 0)
			rootNode = model.nodes[model.skins[0].skeleton];

		for (const tinygltf::Node& node : model.nodes)
		{
			BoneInfo bone = {};
			bone.children = node.children;
			skinnedMesh->AddBone(bone);
		}

		const tinygltf::Scene& scene = model.scenes[model.defaultScene > -1 ? model.defaultScene : 0];
		for (int32_t nodeIndex : scene.nodes)
		{
			LoadNode(nullptr, nodeIndex, model, skinnedMesh);
		}
		//Load Nodes 
		//TODO: Load these in recursive following https://github.com/SaschaWillems/Vulkan-glTF-PBR/blob/master/base/VulkanglTFModel.hpp line 658
		/*for (const tinygltf::Node& node : model.nodes)
		{

			glm::vec3 position = node.translation.size() == 0 ? glm::vec3(0.0f) : glm::make_vec3(node.translation.data());
			glm::vec3 scale = node.scale.size() == 0 ? glm::vec3(1.0f) : glm::make_vec3(node.scale.data());
			glm::quat rotation = node.rotation.size() == 0 ? glm::vec4(0.0f) : glm::make_vec4(node.rotation.data());

			glm::mat4 transform = node.matrix.size() == 16 ? glm::make_mat4x4(node.matrix.data()) : glm::mat4();

			BoneInfo bone = {};
			bone.children = node.children;
			bone.localTransform = (glm::translate(position) * glm::toMat4(rotation) * glm::scale(scale)) * transform;

			//bone.localTransform = glm::translate(glm::mat4(1.0f), position) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale);

			//bone.localTransform = glm::scale(scale) * glm::toMat4(rotation) * glm::translate(position);
			bone.finalTransformation = bone.localTransform;
			skinnedMesh->AddBone(bone);
		}*/


		std::vector<Animation> animations;
		animations.reserve(model.animations.size());

		//Load Animations
		for (const tinygltf::BoneAnimation& gltfAnimation : model.animations)
		{
			std::string name = gltfAnimation.name;
			const  std::vector<tinygltf::AnimationChannel>& channels = gltfAnimation.channels;
			const  std::vector<tinygltf::AnimationSampler>& samplers = gltfAnimation.samplers;

			assert((gltfAnimation.channels.size() % 3) == 0);

			std::vector<float> keyFrames;
			std::vector<BoneAnimation> boneAnimations;
			boneAnimations.reserve(gltfAnimation.channels.size() / 3);
									
			for (const tinygltf::AnimationChannel& channel : gltfAnimation.channels)
			{
				if (channel.target_node < 0)
					continue;

				const tinygltf::Node& targetNode = model.nodes[channel.target_node];

				const tinygltf::AnimationSampler& sampler = samplers[channel.sampler];
				
				const tinygltf::Accessor& timePointAccessor = model.accessors[sampler.input];
				const tinygltf::Accessor& frameValueAccessor = model.accessors[sampler.output];
				assert(timePointAccessor.count == frameValueAccessor.count);

				const tinygltf::BufferView& inputBufferView = model.bufferViews[timePointAccessor.bufferView];
				const tinygltf::BufferView& outputBufferView = model.bufferViews[frameValueAccessor.bufferView];

				const tinygltf::Buffer& inputBuffer = model.buffers[inputBufferView.buffer];
				const tinygltf::Buffer& outputBuffer = model.buffers[outputBufferView.buffer];

				const float* inputArray = reinterpret_cast<const float *>(&(inputBuffer.data[timePointAccessor.byteOffset + inputBufferView.byteOffset]));
				const float* outputArray = reinterpret_cast<const float *>(&(outputBuffer.data[frameValueAccessor.byteOffset + outputBufferView.byteOffset]));

				BoneAnimation* animation = nullptr;

				for (BoneAnimation& existingAnimation : boneAnimations)
				{
					if (existingAnimation.targetBone == channel.target_node)
					{
						animation = &existingAnimation;
						break;
					}
				}

				if (animation == nullptr)
				{
					boneAnimations.emplace_back(BoneAnimation{});
					animation = &boneAnimations[boneAnimations.size() - 1];

					animation->keyFrames.reserve(timePointAccessor.count);
					animation->translations.reserve(frameValueAccessor.count);
					animation->scales.reserve(frameValueAccessor.count);
					animation->rotations.reserve(frameValueAccessor.count);

					for (int i = 0; i < timePointAccessor.count; i++)
					{
						animation->keyFrames.emplace_back(inputArray[i]);
					}

					animation->name = gltfAnimation.name;
					animation->name += "( target_node: " + std::to_string(channel.target_node) + ")";

					animation->targetBone = channel.target_node;

					
					if (sampler.interpolation.find("LINEAR") != std::string::npos)
						animation->interpolationMode = Interpolation::Linear;
					/*else if (sampler.interpolation.find("STEP") != std::string::npos)
						animation->interpolationMode = Interpolation::Step;
					else if (sampler.interpolation.find("CUBICSPLINE") != std::string::npos)
						animation->interpolationMode = Interpolation::CubicSpline;*/
					else
						assert(false && "Unsupported interpolation method");

				}				

				if (channel.target_path.find("translation") != std::string::npos)
				{
					for (int i = 0; i < frameValueAccessor.count; i++)
					{
						animation->translations.emplace_back(glm::make_vec3(&outputArray[i * 3]));
					}
				}
				else if (channel.target_path.find("scale") != std::string::npos)
				{

					for (int i = 0; i < frameValueAccessor.count; i++)
					{
						animation->scales.emplace_back(glm::make_vec3(&outputArray[i * 3]));
					}
				}
				else if(channel.target_path.find("rotation") != std::string::npos)
				{
					for (int i = 0; i < frameValueAccessor.count; i++)
					{
						glm::vec4 input = glm::make_vec4(&outputArray[i * 4]);
						glm::quat quaternion;
						quaternion.x = input.x;
						quaternion.y = input.y;
						quaternion.z = input.z;
						quaternion.w = input.w;

						animation->rotations.emplace_back(quaternion);
					}
				}
				else
				{
					assert(false && "Unsupported target path");
				}

			}

			animations.emplace_back(boneAnimations);
		}

		skinnedMesh->SetAnimation(animations);

		//Load Skins
		for (const tinygltf::Skin& source : model.skins)
		{
			SkinInfo skinInfo = {};
			skinInfo.name = source.name;

			// Find skeleton root node
			if (source.skeleton > -1) 
			{
				skinInfo.rootBone = source.skeleton;
			}

			skinInfo.joints = source.joints;

			//std::vector<int32_t> boneOrder;
			//boneOrder.reserve(source.joints.size());
			//TraverseBones(source.skeleton, skinnedMesh->bones, boneOrder);
			//assert(boneOrder == source.joints);

			// Get inverse bind matrices from buffer
			if (source.inverseBindMatrices > -1) 
			{
				constexpr int mat4Size = sizeof(glm::mat4);

				const tinygltf::Accessor& accessor = model.accessors[source.inverseBindMatrices];
				const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

				skinInfo.inverseBindMatrices.resize(accessor.count);
				memcpy(skinInfo.inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * mat4Size);

				//int skinJointIndex = 0;
				//for (int jointId : source.joints)
				//{
				//	skinnedMesh->bones[jointId].inverseBindMatrix = inverseBindMatrices[skinJointIndex];
				//	skinJointIndex++;
				//}
			}


			skinnedMesh->AddSkin(skinInfo);
		}

	}

}

void MeshFileLoader::GLTFStaticMesh(const tinygltf::Model* model, std::shared_ptr<Mesh> meshDestination)
{
	for (const auto& mesh : model->meshes)
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

			int32_t vertexCount = 0;
			const float* bufferPos = nullptr;
			const float* bufferNormals = nullptr;
			const float* bufferTexCoords = nullptr;
			const uint16_t* bufferJoints = nullptr;
			const float* bufferWeights = nullptr;

			assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

			{
				const tinygltf::Accessor &posAccessor = model->accessors[primitive.attributes.find("POSITION")->second];
				const tinygltf::BufferView &posView = model->bufferViews[posAccessor.bufferView];
				bufferPos = reinterpret_cast<const float *>(&(model->buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
				vertexCount = posAccessor.count;
				assert(posAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
			}

			if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
			{
				const tinygltf::Accessor &normAccessor = model->accessors[primitive.attributes.find("NORMAL")->second];
				const tinygltf::BufferView &normView = model->bufferViews[normAccessor.bufferView];
				bufferNormals = reinterpret_cast<const float *>(&(model->buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
				assert(normAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
			}

			if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) 
			{
				const tinygltf::Accessor &uvAccessor = model->accessors[primitive.attributes.find("TEXCOORD_0")->second];
				const tinygltf::BufferView &uvView = model->bufferViews[uvAccessor.bufferView];
				bufferTexCoords = reinterpret_cast<const float *>(&(model->buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
				assert(uvAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
			}

			if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end())
			{
				const tinygltf::Accessor &jointAccessor = model->accessors[primitive.attributes.find("JOINTS_0")->second];
				const tinygltf::BufferView &jointView = model->bufferViews[jointAccessor.bufferView];
				bufferJoints = reinterpret_cast<const uint16_t *>(&(model->buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]));

				assert(jointAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);
			}

			if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end())
			{
				const tinygltf::Accessor &weightAccessor = model->accessors[primitive.attributes.find("WEIGHTS_0")->second];
				const tinygltf::BufferView &weightView = model->bufferViews[weightAccessor.bufferView];
				bufferWeights = reinterpret_cast<const float *>(&(model->buffers[weightView.buffer].data[weightAccessor.byteOffset + weightView.byteOffset]));

				assert(weightAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
			}

			bool hasSkin = (bufferJoints && bufferWeights);

			for (size_t v = 0; v < vertexCount; v++)
			{
				Vertex vertex{};
				vertex.pos = glm::make_vec3(&bufferPos[v * 3]);
				vertex.normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * 3]) : glm::vec3(0.0f)));
				vertex.texCoords = bufferTexCoords ? glm::make_vec2(&bufferTexCoords[v * 2]) : glm::vec2(0.0f);
				vertex.color = glm::vec4(1.0f);

				vertex.joint0 = hasSkin ? glm::vec4(glm::make_vec4(&bufferJoints[v * 4])) : glm::vec4(0.0f);
				vertex.weight0 = hasSkin ? glm::make_vec4(&bufferWeights[v * 4]) : glm::vec4(0.0f);
				vertices.emplace_back(vertex);

				aabb.Grow(vertex.pos);
			}



			// Indices
			{
				const tinygltf::Accessor &accessor = model->accessors[primitive.indices];
				const tinygltf::BufferView &bufferView = model->bufferViews[accessor.bufferView];
				const tinygltf::Buffer &buffer = model->buffers[bufferView.buffer];

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
			SubMesh* subMesh = meshDestination->AllocateBuffers(static_cast<void*>(&vertices[0]), sizeof(Vertex) * vertices.size(), static_cast<void*>(&indices[0]), sizeof(uint32_t) * indices.size(), triangleCount);
			subMesh->SetAABB(aabb);
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
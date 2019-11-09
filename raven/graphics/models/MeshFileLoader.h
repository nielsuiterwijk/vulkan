#pragma once

#include "graphics/helpers/InstanceWrapper.h"


class Mesh;
class SkinnedMesh;
struct BoneInfo;

namespace tinygltf
{
class Model;
class Node;
};

class MeshFileLoader
{
public:
	struct MeshFileType
	{
		enum Enum
		{
			OBJ,
			STL,
			GLTF
		};
	};

	static std::shared_ptr<Mesh> Static( const std::string& fileName );
	static std::shared_ptr<Mesh> Skinned( const std::string& fileName );

private:
	static void FileLoaded( std::vector<char> fileData, std::shared_ptr<Mesh> mesh, MeshFileType::Enum fileType );

	static void LoadNode( BoneInfo* parent, uint32_t nodeIndex, const tinygltf::Model& model, std::shared_ptr<SkinnedMesh> skinnedMesh );
	static void LoadGLTF( std::vector<char>& fileData, std::shared_ptr<Mesh> meshDestination );
	static void GLTFStaticMesh( const tinygltf::Model* model, std::shared_ptr<Mesh> meshDestination );

	static void LoadOBJ( std::vector<char>& fileData, std::shared_ptr<Mesh> mesh );
	static void LoadSTL( const std::vector<char>& fileData, std::shared_ptr<Mesh> mesh );

protected:
};

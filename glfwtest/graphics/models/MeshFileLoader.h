#pragma once

#include "standard.h"
#include "graphics/helpers/InstanceWrapper.h"


class Mesh;

namespace tinygltf
{
	class Model;
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
	
	static std::shared_ptr<Mesh> Static(const std::string& fileName);
	static std::shared_ptr<Mesh> Skinned(const std::string& fileName);

private:
	static void FileLoaded(std::vector<char> fileData, std::shared_ptr<Mesh> mesh, MeshFileType::Enum fileType);

	static void LoadGLTF(std::vector<char>& fileData, std::shared_ptr<Mesh> meshDestination);
	static void GLTFStaticMesh(const tinygltf::Model* model, std::shared_ptr<Mesh> meshDestination);


	static void LoadOBJ(std::vector<char>& fileData, std::shared_ptr<Mesh> mesh);
	static void LoadSTL(const std::vector<char>& fileData, std::shared_ptr<Mesh> mesh);

protected:
};
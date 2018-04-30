#pragma once

#include "standard.h"
#include "graphics/helpers/InstanceWrapper.h"

class Mesh;

class MeshFileLoader
{
public:
	struct MeshFileType
	{
		enum Enum
		{
			OBJ,
			STL
		};
	};

	MeshFileLoader();
	~MeshFileLoader();

	static std::shared_ptr<Mesh> Get(const std::string& fileName);

private:
	static void FileLoaded(std::vector<char> fileData, std::shared_ptr<Mesh> mesh, MeshFileType::Enum fileType);

	static void LoadOBJ(std::vector<char>& fileData, std::shared_ptr<Mesh> mesh);
	static void LoadSTL(const std::vector<char>& fileData, std::shared_ptr<Mesh> mesh);

protected:
};
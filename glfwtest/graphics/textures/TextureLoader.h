#pragma once

#include "standard.h"
#include "graphics/helpers/InstanceWrapper.h"

class Texture2D;

class TextureLoader
{
public:
	TextureLoader();
	~TextureLoader();
	
	static std::shared_ptr<Texture2D> Get(const std::string& fileName);

private:
	static void FileLoaded(std::vector<char> fileData);

protected:

};
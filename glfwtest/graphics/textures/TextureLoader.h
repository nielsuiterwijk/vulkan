#pragma once

#include "graphics/helpers/InstanceWrapper.h"
#include "standard.h"

class Texture2D;

class TextureLoader
{
public:
	TextureLoader();
	~TextureLoader();

	static std::shared_ptr<Texture2D> Get( const std::string& fileName );

private:
	static void FileLoaded( std::vector<char> fileData, std::shared_ptr<Texture2D> texture2d );

protected:
};

#include "DebugUI.h"

#include "io/FileUtility.h"

#include "imgui/imgui.h"

#include <filesystem>
#include <iostream>

void DebugUI::ListGameObjects()
{
	//E:\Code\C++\glfwtest\vulkan\assets\gameobjects
	//E:\\Code\\C++\\glfwtest\\vulkan\\bin\\x64
	std::filesystem::path pathToShow( std::filesystem::current_path() );
	std::vector<std::string> filesInDirectory;
	IO::ListFiles( "E:\\Code\\C++\\glfwtest\\vulkan\\assets\\gameobjects", filesInDirectory );

	static auto vector_getter = []( void* vec, int idx, const char** out_text ) {
		auto& vector = *static_cast<std::vector<std::string>*>( vec );
		if ( idx < 0 || idx >= static_cast<int>( vector.size() ) )
		{
			return false;
		}
		*out_text = vector.at( idx ).c_str();
		return true;
	};

	static int32_t index = 0;

	ImGui::Begin( "Entities" );
	ImGui::ListBox( "files", &index, vector_getter, static_cast<void*>( &filesInDirectory ), filesInDirectory.size() );
	if ( ImGui::Button( "Spawn" ) )
	{
		//std::shared_ptr<Model> model = std::make_shared<Model>( "cesiumman" );
		//models.push_back( model );
		std::cout << "Spawned " << filesInDirectory[ index ] << std::endl;
	}
	ImGui::End();
}

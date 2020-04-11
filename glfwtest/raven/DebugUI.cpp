#include "DebugUI.h"

#include "imgui/imgui.h"
#include "io/FileUtility.h"

#include <filesystem>
#include <iostream>

void DebugUI::ListGameObjects()
{
	//E:\Code\C++\glfwtest\vulkan\assets\gameobjects
	//E:\\Code\\C++\\glfwtest\\vulkan\\bin\\x64
	std::filesystem::path pathToShow( std::filesystem::current_path() );
	std::vector<std::string> filesInDirectory;
	IO::ListFiles( "C:\\projects\\cpp\\vulkan\\assets\\gameobjects", filesInDirectory );

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
	ImGui::ListBox( "files", &index, vector_getter, static_cast<void*>( &filesInDirectory ), static_cast<int>( filesInDirectory.size() ), 4 );
	if ( ImGui::Button( "Spawn" ) )
	{
		//std::shared_ptr<Model> model = std::make_shared<Model>( "cesiumman" );
		//models.push_back( model );
		std::cout << "Spawned " << filesInDirectory[ index ] << std::endl;
	}
	ImGui::End();
}

void DebugUI::TimingGraph( float CpuTimeInSeconds, float FrameTimeInSeconds ) //TODO: instead of float, use chrono?
{
	static std::vector<float> CPUFrames; //TODO: Use a circular buffer?
	static std::vector<float> GPUFrames;

	if ( CPUFrames.size() >= 100 ) //Max seconds to show
	{
		for ( size_t i = 1; i < CPUFrames.size(); i++ )
		{
			CPUFrames[ i - 1 ] = CPUFrames[ i ];
			GPUFrames[ i - 1 ] = GPUFrames[ i ];
		}

		CPUFrames[ CPUFrames.size() - 1 ] = CpuTimeInSeconds * 1000;
		GPUFrames[ GPUFrames.size() - 1 ] = FrameTimeInSeconds * 1000;
	}
	else
	{
		CPUFrames.push_back( CpuTimeInSeconds );
		GPUFrames.push_back( FrameTimeInSeconds );
	}

	ImGui::Begin( "Timings" );

	char text[ 20 ];

	sprintf_s( text, 20, "CPU: %.03f", CpuTimeInSeconds * 1000 );
	ImGui::PlotLines( text, &CPUFrames[ 0 ], CPUFrames.size(), 0, nullptr, 0.0f, 50.0f, ImVec2( 100, 50 ) );

	sprintf_s( text, 20, "GPU: %.03f", FrameTimeInSeconds * 1000 ); //Technically not GPU, but RenderFrame time..
	ImGui::PlotLines( text, &GPUFrames[ 0 ], GPUFrames.size(), 0, nullptr, 0.0f, 33.0f, ImVec2( 100, 50 ) );

	ImGui::End();
}

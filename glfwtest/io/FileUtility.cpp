#include "FileUtility.h"

void IO::ListFiles( const std::string& Directory, std::vector<std::string>& FilesOut )
{
	if ( std::filesystem::exists( Directory ) == false || std::filesystem::is_directory( Directory ) == false )
	{
		return;
	}
	//namespace fs = std::filesystem;
	//
	////https://www.codingame.com/playgrounds/5659/c17-filesystem
	//fs::path pathToShow( fs::current_path() );
	//std::cout << "exists() = " << fs::exists( pathToShow ) << "\n"
	//		  << "root_name() = " << pathToShow.root_name() << "\n"
	//		  << "root_path() = " << pathToShow.root_path() << "\n"
	//		  << "relative_path() = " << pathToShow.relative_path() << "\n"
	//		  << "parent_path() = " << pathToShow.parent_path() << "\n"
	//		  << "filename() = " << pathToShow.filename() << "\n"
	//		  << "stem() = " << pathToShow.stem() << "\n"
	//		  << "extension() = " << pathToShow.extension() << "\n";
	//
	//int i = 0;

	for ( const auto& entry : std::filesystem::directory_iterator( Directory ) )
	{
		std::filesystem::path filename = entry.path().filename();
		if ( std::filesystem::is_directory( entry.status() ) )
		{
			std::cout << "[+] " << filename << "\n";
		}
		else if ( std::filesystem::is_regular_file( entry.status() ) )
		{
			FilesOut.emplace_back( filename.string() );
		}
		else
		{
			assert( false && "invalid file" );
		}
	}
}

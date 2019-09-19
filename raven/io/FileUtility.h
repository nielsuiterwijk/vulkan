#pragma once

#include <string>
#include <vector>

namespace IO
{
void ListFiles( const std::string& Directory, std::vector<std::string>& FilesOut );
}

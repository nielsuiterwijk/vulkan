#include "InputEvent.h"


std::vector<std::function<void( int, int, int )>> InputEvent::OnMouseButton = {};
std::vector<std::function<void( double, double )>> InputEvent::OnMouseScroll = {};
std::vector<std::function<void( int, int, int, int )>> InputEvent::OnKey = {};
std::vector<std::function<void( unsigned int )>> InputEvent::OnChar = {};
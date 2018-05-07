#include "Color.h"

Color Color::Red = Color(230, 25, 75, 255);
Color Color::Green = Color(60, 180, 75, 255);
Color Color::Yellow = Color(255, 225, 25, 255);
Color Color::Blue = Color(0, 130, 200, 255);
Color Color::Orange = Color(245, 130, 48, 255);
Color Color::Purple = Color(145, 30, 180, 255);
Color Color::Cyan = Color(70, 240, 240, 255);
Color Color::Magenta = Color(240, 50, 230, 255);
Color Color::Lime = Color(210, 245, 60, 255);
Color Color::Pink = Color(250, 190, 190, 255);
Color Color::Teal = Color(0, 128, 128, 255);
Color Color::Lavender = Color(230, 190, 255, 255);
Color Color::Brown = Color(170, 110, 40, 255);
Color Color::Beige = Color(255, 250, 200, 255);
Color Color::Maroon = Color(128, 0, 0, 255);
Color Color::Mint = Color(170, 255, 195, 255);
Color Color::Olive = Color(128, 128, 0, 255);
Color Color::Coral = Color(255, 215, 180, 255);
Color Color::Navy = Color(0, 0, 128, 255);
Color Color::Grey = Color(128, 128, 128, 255);
Color Color::White = Color(255, 255, 255, 255);
Color Color::Black = Color(0, 0, 0, 255);


Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) :
	Values(r, g, b, a)
{

}

Color::~Color()
{

}
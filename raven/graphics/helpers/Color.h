#pragma once



class Color
{
public:
	Color( uint8_t r, uint8_t g, uint8_t b, uint8_t a );
	~Color();

	glm::u8vec4 Values;

	operator glm::vec3()
	{
		return glm::vec3( (float)Values.r / 255.0f, (float)Values.g / 255.0f, (float)Values.b / 255.0f );
	}

	operator glm::vec4()
	{
		return glm::vec4( (float)Values.r / 255.0f, (float)Values.g / 255.0f, (float)Values.b / 255.0f, (float)Values.a / 255.0f );
	}

public:
	static Color Red, Green, Yellow, Blue, Orange, Purple, Cyan, Magenta, Lime, Pink, Teal, Lavender, Brown, Beige, Maroon, Mint, Olive, Coral, Navy, Grey, White, Black;
};

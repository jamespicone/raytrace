#ifndef __MIRRORHOUSE_COLOUR_H
#define __MIRRORHOUSE_COLOUR_H

#include <cstdint>
#include <algorithm>

struct Colour
{
	uint8_t red;
	uint8_t blue;
	uint8_t green;
	
	uint32_t convertTo32() const {return (red << 24) + (blue << 16) + (green << 8);}
	
	Colour()
	:red(0), blue(0), green(0)
	{}	

	Colour(uint32_t colour)
	{
		red = (colour & 0xff000000u) >> 24;
		blue = (colour & 0x00ff0000u) >> 16;
		green = (colour & 0x0000ff00u) >> 8;
	}
	
	Colour(uint8_t _red, uint8_t _blue, uint8_t _green)
	:red(_red), blue(_blue), green(_green)
	{}
	
	Colour operator*(double factor) const
	{
		return Colour(
			std::min(std::max(red * factor, 0.0), 255.0),
			std::min(std::max(blue * factor, 0.0), 255.0),
			std::min(std::max(green * factor, 0.0), 255.0)
		);
	}
	
	Colour& operator*=(double factor)
	{
		return *this = *this * factor;
	}
	
	Colour operator+(const Colour& rhs) const
	{
		return Colour(std::min(std::max(red + rhs.red, 0), 255), std::min(std::max(blue + rhs.blue, 0), 255), std::min(std::max(green + rhs.green, 0), 255));
	}
	
	Colour& operator+=(const Colour& rhs)
	{
		return *this = *this + rhs;
	}
};

#endif
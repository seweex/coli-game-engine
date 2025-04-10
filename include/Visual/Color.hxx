#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

namespace Coli
{
	namespace Visual
	{
		struct alignas(4) Color
		{
		public:
			uint8_t r = 0;
			uint8_t g = 0;
			uint8_t b = 0;
			uint8_t a = 255;
		};
	}
}
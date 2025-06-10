#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Color.hxx"

namespace Coli
{
	namespace Visual
	{
		class Texture final
		{
		public:
			Texture (size_t width, size_t height, Color const* data) :
				myColors (data, data + width * height),
				myWidth  (width)
			{}

			Texture (Color color) :
				Texture (1, 1, &color)
			{}

			_NODISCARD std::pair <size_t, size_t> size() const noexcept {
				return { myWidth, myColors.size() / myWidth };
			}

			_NODISCARD Color const* data() const noexcept {
				return myColors.data();
			}

		private:
			std::vector<Color> myColors;
			size_t myWidth;
		};
	}
}
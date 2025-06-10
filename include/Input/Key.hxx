#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

namespace Coli
{
	namespace Input
	{
		enum class Action {
			pressed,
			released,
			hovered,
			double_click
		};

		enum class KeyboardButton {};

		enum class MouseButton {
			left, 
			middle, 
			right, 
			x1, 
			x2
		};

		class Button
		{
		public:
			Button (KeyboardButton button) noexcept :
				myValue (button)
			{}

			Button (MouseButton button) noexcept :
				myValue (button)
			{}

			_NODISCARD bool operator==(Button const& other) const noexcept 
			{
				if (myValue.index() == other.myValue.index())
					switch (myValue.index())
					{
					case 0:
						return std::get<0>(myValue) == std::get<0>(other.myValue);

					case 1:
						return std::get<1>(myValue) == std::get<1>(other.myValue);

					default:
						return false;
					}
			}

		private:
			std::variant <KeyboardButton, MouseButton> myValue;
		};
	}
}
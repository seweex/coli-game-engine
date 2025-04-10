#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

namespace Coli
{
	namespace Detail
	{
		enum class MouseButton {
			left, 
			middle, 
			right, 
			x1, 
			x2
		};

		enum class Action {
			no_action,
			pressed,
			released,
			hovered,
			double_click
		};

		enum class MouseAxisDependence {
			wheel,
			cursor_x,
			cursor_y
		};

		class Button final
		{
		public:
			Button(int key) noexcept :
				myKey (key)
			{}

			Button(MouseButton key) noexcept :
				myKey (key)
			{}

			_NODISCARD bool does_match(auto key) const noexcept 
				requires (std::_Is_any_of_v<decltype(key), int, MouseButton>)
			{
				if (std::holds_alternative<decltype(key)>(myKey))
					if (key == std::get<decltype(key)>(myKey))
						return true;

				return false;
			}

			_NODISCARD bool is_used() const noexcept {
				return isUsed;
			}

			void activate() noexcept {
				isUsed = true;
			}
			
			void deactivate() noexcept {
				isUsed = false;
			}

		private:
			std::variant<int, MouseButton> myKey;
			bool isUsed = false;
		};

		class AxisBase
		{
		public:
			_NODISCARD double get_value() const noexcept {
				return myValue;
			}

			void reset() noexcept {
				myValue = 0;
			}

		protected:
			double myValue = 0;
 		};
	}

	namespace Input
	{
		class KeyAxis final :
			public Detail::AxisBase
		{
		public:
			KeyAxis (
				double activeVal,
				Detail::Button positive,
				Detail::Button negative,
				std::optional<Detail::Button> altPositive = std::nullopt,
				std::optional<Detail::Button> altNegative = std::nullopt
			) noexcept :
				myActiveValue (activeVal),
				myPosKey	  (positive),
				myNegKey	  (negative),
				myAltPosKey   (altPositive),
				myAltNegKey	  (altNegative)
			{}

			void handle_input(auto key, Detail::Action action) noexcept
				requires (std::_Is_any_of_v<decltype(key), int, Detail::MouseButton>)
			{
				auto handle = [&](Detail::Button& btn, bool inverse)
				{
					if (btn.does_match(key)) _UNLIKELY
					{
						if (action == Detail::Action::released && btn.is_used()) _UNLIKELY {
							inverse ? myValue += myActiveValue : myValue -= myActiveValue;
							btn.deactivate();
						}
						if (action == Detail::Action::pressed && !btn.is_used()) _UNLIKELY {
							inverse ? myValue -= myActiveValue : myValue += myActiveValue;
							btn.activate();
						}
					}
				};

				handle(myPosKey, false);
				handle(myNegKey, true);

				if (myAltPosKey) handle(*myAltPosKey, false);
				if (myAltNegKey) handle(*myAltNegKey, true);
			}

		private:
			using Detail::AxisBase::myValue;

			double const myActiveValue;

			std::optional<Detail::Button> myAltPosKey;
			std::optional<Detail::Button> myAltNegKey;

			Detail::Button myPosKey;
			Detail::Button myNegKey;

		};

		class MouseAxis final :
			public Detail::AxisBase
		{
		public:
			MouseAxis(double sensitivity, Detail::MouseAxisDependence dependence) noexcept :
				mySensitivity (sensitivity),
				myDependence  (dependence)
			{}
			
			void handle_input(double cursorX, double cursorY) noexcept
			{
				switch (myDependence)
				{
				case Detail::MouseAxisDependence::cursor_x:
					myValue = mySensitivity * cursorX;
					break;

				case Detail::MouseAxisDependence::cursor_y:
					myValue = mySensitivity * cursorY;
					break;				
				}
			}

			void handle_input(double wheel) noexcept {
				if (myDependence == Detail::MouseAxisDependence::wheel) _UNLIKELY
					myValue += mySensitivity * wheel;
			}

		private:
			using Detail::AxisBase::myValue;

			double const					  mySensitivity;
			Detail::MouseAxisDependence const myDependence;
		};
	}
}
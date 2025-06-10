#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Key.hxx"
#include "Event.hxx"

namespace Coli
{
	namespace Input
	{
		class ButtonEvent;
		class MouseEvent;
	}

	namespace Detail
	{
		class AxisBase
		{
		protected:
			AxisBase() noexcept = default;

		public:
			_NODISCARD virtual double get_value() const noexcept = 0;

			virtual void handle (Input::ButtonEvent const& event) noexcept {}
			virtual void handle (Input::MouseEvent const& event)  noexcept {}
		};
	}

	namespace Input
	{
		class ButtonAxis final :
			public Detail::AxisBase
		{
		public:
			ButtonAxis (
				Button const& positive,
				Button const& negative
			) noexcept :
				myPositive (positive),
				myNegative (negative)
			{}

			ButtonAxis(ButtonAxis&&)	  = default;
			ButtonAxis(ButtonAxis const&) = default;

			ButtonAxis& operator=(ButtonAxis&&)		 = default;
			ButtonAxis& operator=(ButtonAxis const&) = default;

			_NODISCARD double get_value() const noexcept override
			{
				double value = 0;

				if (myPositivePressed) value += 1;
				if (myNegativePressed) value -= 1;

				return value;
			}

			void handle (Input::ButtonEvent const& event) noexcept 
			{
				auto handle_for = [&] (Button const& key, bool& flag) 
				{
					if (key == event.get_button())
						switch (event.get_action())
						{
						case Action::hovered: _FALLTHROUGH;
						case Action::pressed: _FALLTHROUGH;
						case Action::double_click:
							flag = true;
							break;

						case Action::released:
							flag = false;
							break;
						}
				};

				handle_for (myPositive, myPositivePressed);
				handle_for (myNegative, myNegativePressed);
			}

		private:
			Button myPositive;
			Button myNegative;

			bool myPositivePressed = false;
			bool myNegativePressed = false;
		};

		class MouseAxis final :
			public Detail::AxisBase
		{
		public:
			enum class Dependence {
				cursor_x,
				cursor_y,
				wheel
			};

			MouseAxis (double sensitivity, Dependence dependence) noexcept :
				mySensitivity (sensitivity),
				myDependence  (dependence)
			{}

			MouseAxis(MouseAxis&&)	    = default;
			MouseAxis(MouseAxis const&) = default;

			MouseAxis& operator=(MouseAxis&&)	   = default;
			MouseAxis& operator=(MouseAxis const&) = default;

			_NODISCARD double get_value() const noexcept override {
				return myValue;
			}

			void handle (Input::MouseEvent const& event) noexcept final 
			{
				switch (myDependence)
				{
				case Dependence::wheel:
					if (auto value = event.get_wheel())
						myValue += *value * mySensitivity;

					break;

				case Dependence::cursor_x:
					if (auto value = event.get_cursor())
						myValue += value->first * mySensitivity;

					break;

				case Dependence::cursor_y:
					if (auto value = event.get_cursor())
						myValue += value->second * mySensitivity;

					break;
				}
			}

		private:
			Dependence myDependence;

			double mySensitivity;
			double myValue = 0;
		};
	}
}
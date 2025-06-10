#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Key.hxx"

namespace Coli
{
	namespace Detail
	{
		class InputEventBase
		{
		protected:
			InputEventBase() noexcept = default;

		public:
			static constexpr size_t max_categories = 2;
			
			enum class Category {
				button,
				mouse
			};

			_NODISCARD virtual Category get_category() const noexcept = 0;
		};
	}

	namespace Input
	{
		class ButtonEvent final :
			public Detail::InputEventBase
		{
		public:
			using InputEventBase::Category;

			ButtonEvent (Button const& button, Action action) noexcept :
				myButton (button),
				myAction (action)
			{}
			
			_NODISCARD Category get_category() const noexcept final {
				return Category::button;
			}

			_NODISCARD Button const& get_button() const noexcept {
				return myButton;
			}

			_NODISCARD Action get_action() const noexcept {
				return myAction;
			}

		private:
			Button myButton;
			Action myAction;
		};

		class MouseEvent final :
			public Detail::InputEventBase
		{
		public:
			using Detail::InputEventBase::Category;

			MouseEvent (double wheel) noexcept :
				myValue (wheel)
			{}

			MouseEvent (double cursorX, double cursorY) noexcept :
				myValue (std::make_pair(cursorX, cursorY))
			{}

			_NODISCARD Category get_category() const noexcept final {
				return Category::mouse;
			}

		private:
			template <int _Idx>
			_NODISCARD auto extract() const noexcept 
			{
				using type = std::conditional_t <_Idx == 0, std::pair<double, double>, double>;

				if (std::holds_alternative<type>(myValue))
					return std::make_optional(std::get<type>(myValue));
				else
					return std::optional<type>(std::nullopt);
			}

		public:
			_NODISCARD std::optional <std::pair<double, double>> get_cursor() const noexcept {
				return extract<0>();
			}

			_NODISCARD std::optional <double> get_wheel() const noexcept {
				return extract<1>();
			}

		private:
			std::variant <std::pair <double, double>, double> myValue;
		};
	}
}
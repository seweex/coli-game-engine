#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Axis.hxx"

namespace Coli
{
	namespace Input
	{
		class Map final 
		{
		public:
			_NODISCARD double get_axis_value(std::string_view name) const 
			{
				auto iter = myAxes.find(name);

				if (iter != myAxes.end())
					return iter->second->get_value();
				else 
					throw std::invalid_argument("no axis with this name");
			}

			void make_axis(
				std::string_view name,
				Detail::Button positive,
				Detail::Button negative,
				double activeVal = 1,
				std::optional<Detail::Button> altPositive = std::nullopt,
				std::optional<Detail::Button> altNegative = std::nullopt
			) {
				auto [iter, inserted] = myAxes.try_emplace(std::string(name), nullptr);

				if (inserted) {
					auto ptr = std::make_shared<KeyAxis>(activeVal, positive, negative, altPositive, altNegative);

					iter->second = ptr;
					myKeyAxes.insert(ptr);
				}
				else
					throw std::runtime_error("another axis with the same name already exist");
			}

			void make_axis(std::string_view name, double sensitivity, Detail::MouseAxisDependence dependence)
			{
				auto [iter, inserted] = myAxes.try_emplace(std::string(name), nullptr);

				if (inserted) {
					auto ptr = std::make_shared<MouseAxis>(sensitivity, dependence);

					iter->second = ptr;
					myMouseAxes.insert(ptr);
				}
				else
					throw std::runtime_error("another axis with the same name already exist");
			}

			void handle_input(double cursorX, double cursorY) noexcept  {
				for (auto& mouseAxis : myMouseAxes)
					mouseAxis->handle_input(cursorX, cursorY);
			}

			void handle_input(double wheel) noexcept {
				for (auto& mouseAxis : myMouseAxes)
					mouseAxis->handle_input(wheel);
			}

			void handle_input(int key, Detail::Action action) noexcept {
				for (auto& keyAxis : myKeyAxes)
					keyAxis->handle_input(key, action);
			}

			void handle_input(Detail::MouseButton button, Detail::Action action) noexcept {
				for (auto& keyAxis : myKeyAxes)
					keyAxis->handle_input(button, action);
			}

		private:
			std::unordered_map<std::string, std::shared_ptr<Detail::AxisBase>, 
							   Detail::TransparentHash, std::equal_to<>>
			myAxes;

			std::unordered_set<std::shared_ptr<KeyAxis>>   myKeyAxes;
			std::unordered_set<std::shared_ptr<MouseAxis>> myMouseAxes;
		};
	}
}
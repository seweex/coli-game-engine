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
			static void x_no_axis_with_name() {
				throw std::invalid_argument("There are no axes with this name");
			}

			static void x_axis_exists() {
				throw std::invalid_argument("Another axis with the same name already exists");
			}

		public:
			Map() = default;

			Map(Map&&)		= delete;
			Map(Map const&) = delete;

			Map& operator=(Map&&)	   = delete;
			Map& operator=(Map const&) = delete;

			template <std::derived_from <Detail::AxisBase> _Ty, class... _ArgTys>
				requires (std::constructible_from <_Ty, _ArgTys...>)
			void add_axis (std::string_view name, _ArgTys&&... args) 
			{
				if (myAxes.contains(name))
					x_axis_exists();

				else {
					auto ptr = std::make_shared<_Ty>(std::forward<_ArgTys>(args)...);
					myAxes.emplace(name, std::move(ptr));
				}
			}

			void remove_axis (std::string_view name) noexcept {
				myAxes.erase(name);
			}

			_NODISCARD double get_value (std::string_view axisName) const noexcept
			{
				auto iter = myAxes.find(axisName);

				if (iter != myAxes.end())
					return iter->second->get_value();
			}

			template <std::derived_from <Detail::InputEventBase> _Ty>
				requires (!std::same_as <_Ty, Detail::InputEventBase>)
			void handle (_Ty const& event) noexcept {
				for (auto& [_, axis] : myAxes)
					axis->handle(event);
			}

		private:
			std::unordered_map <std::string, 
								std::unique_ptr <Detail::AxisBase>, 
								Detail::TransparentHash, 
								std::equal_to<>>
			myAxes;
		};
	}
}
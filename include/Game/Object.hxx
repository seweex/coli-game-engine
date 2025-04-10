#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Component.hxx"

namespace Coli
{
	namespace Game
	{
		class Object :
			public Detail::EntityBase
		{
		public:
			template <Detail::GameComponent _Ty, class... _ArgTys>
			std::weak_ptr<_Ty> make_component(_ArgTys&&... args)
			{
				auto [iter, inserted] = myComponents.try_emplace(_Ty::get_category());

				if (inserted) _LIKELY 
				{
					auto ptr = std::make_shared<_Ty>(std::forward<_ArgTys>(args)...);

					ptr->set_owner(*this);
					iter->second = ptr;

					return ptr;
				} 
				else {
					if (auto ptr = std::dynamic_pointer_cast<_Ty>(iter->second))
						return ptr;
					else
						throw std::runtime_error("the another incompatible component is already exist");
				}
			}

			template <Detail::GameComponent _Ty>
			_NODISCARD std::weak_ptr<_Ty> get_component()
			{
				auto ptr = try_get_component<_Ty>();
				
				if (!ptr.expired()) _LIKELY
					return ptr;
				else
					throw std::runtime_error("no compatible component with this _Ty");
			}

			template <Detail::GameComponent _Ty>
			_NODISCARD std::weak_ptr<_Ty> try_get_component() noexcept
			{
				auto iter = myComponents.find(_Ty::get_category());

				if (iter != myComponents.end()) _LIKELY
					if (auto ptr = std::dynamic_pointer_cast<_Ty>(iter->second)) _LIKELY
						return ptr;
				
				return {};
			}

			void change_layer(size_t layer) noexcept {
				myPreviousLayer = myLayer;
				myLayer         = layer;
			}

			void reset_layer_change() noexcept {
				myPreviousLayer = myLayer;
			}

			_NODISCARD size_t get_layer() const noexcept {
				return myLayer;
			}

			_NODISCARD bool has_layer_changed() const noexcept {
				return myLayer != myPreviousLayer;
			}

			void on_start() override {
				for (auto& [_, component] : myComponents)
					component->correct_on_start();

				for (auto& [_, component] : myComponents)
					component->take_dependencies();
			}

			void on_update(float time) override {
				for (auto& [_, component] : myComponents)
					component->correct_on_update(time);
			}

			void on_late_update(float time) override {
				for (auto& [_, component] : myComponents)
					component->correct_on_late_update(time);
			}

			void on_render_update() override {
				for (auto& [_, component] : myComponents)
					component->correct_on_render_update();
			}

			_NODISCARD nlohmann::json serialize() const override
			{
				nlohmann::json obj;

				obj[key_id]				 = get_id();
				obj[key_layer]		     = myLayer;
				obj[key_prev_layer]		 = myPreviousLayer;
				obj[key_components_list] = nlohmann::json::array();

				auto& componentsList = obj[key_components_list];

				for (auto const& [_, component] : myComponents)
				{
					nlohmann::json componentObj;

					componentObj[key_data] = component->serialize();
					componentObj[key_id]   = component->get_id();

					componentsList.push_back(componentObj);
				}

				return obj;
			}

			void deserialize(nlohmann::json const& obj) override
			{
				using Detail::Json::try_fill;

				size_t supposedID;
				try_fill (obj, supposedID, key_id);

				if (supposedID == get_id()) _LIKELY
				{
					size_t layer;
					try_fill (obj, layer, key_layer);

					size_t prevLayer;
					try_fill (obj, prevLayer, key_prev_layer);

					nlohmann::json componentsList;
					try_fill (obj, componentsList, key_components_list);

					for (auto const& componentObj : componentsList)
					{
						size_t componentID;
						try_fill (componentObj, componentID, key_id);

						ComponentBase* componentWithID = nullptr;

						for (auto const& [_, component] : myComponents)
							if (component->get_id() == componentID) _UNLIKELY {
								componentWithID = component.get();
								break;
							}

						if (componentWithID) _LIKELY
						{
							nlohmann::json componentData;
							try_fill (componentObj, componentData, key_data);

							componentWithID->deserialize(componentData);
						}
					}
				}
				else 
					throw std::invalid_argument("invalid json object: ID doens't match");
			}

		private:
			static constexpr std::string_view key_components_list = "components";
			static constexpr std::string_view key_id			  = "id";
			static constexpr std::string_view key_data			  = "data";
			static constexpr std::string_view key_layer			  = "layer";
			static constexpr std::string_view key_prev_layer	  = "previousLayer";

			std::unordered_map<Detail::ComponentCategory,
							   std::shared_ptr<ComponentBase>>
			myComponents;

			size_t myLayer		   = 0;
			size_t myPreviousLayer = 0;
		};		
	}
}
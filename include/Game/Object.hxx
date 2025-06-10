#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "../Common/KeepsChange.hxx"

#include "Component.hxx"

namespace Coli
{
	namespace Detail
	{
		class ObjectsContainerBase;

		class LayeredBase :
			public KeepsChangeBase <size_t>
		{
		protected:
			LayeredBase() noexcept {
				set_layer(0);
			}

		public:
			LayeredBase(LayeredBase&&)      = delete;
			LayeredBase(LayeredBase const&) = delete;

			LayeredBase& operator=(LayeredBase&&)      = delete;
			LayeredBase& operator=(LayeredBase const&) = delete;

			_NODISCARD size_t get_layer() const noexcept {
				return myLayer;
			}

			void set_layer(size_t newLayer) noexcept {
				this->commit(myLayer = newLayer);
			}

		private:
			size_t myLayer;
		};

		class ComponentsContainerBase :
			public std::enable_shared_from_this <ComponentsContainerBase>
		{
			static void x_another_incompatible_exists() {
				throw std::runtime_error("Another component with the same category is already exist but it isn't compatible");
			}

			static void x_no_component() {
				throw std::invalid_argument("There are no component of this type");
			}

		protected:
			ComponentsContainerBase() {
				myComponents.reserve(Game::ComponentBase::categories_count);
			}

		public:
			ComponentsContainerBase(ComponentsContainerBase&&)	    = delete;
			ComponentsContainerBase(ComponentsContainerBase const&) = delete;

			ComponentsContainerBase& operator=(ComponentsContainerBase&&)	   = delete;
			ComponentsContainerBase& operator=(ComponentsContainerBase const&) = delete;

			template <GameComponent _Ty, class ... _ArgTys>
				requires std::constructible_from <_Ty, _ArgTys...>
			std::weak_ptr <_Ty> make_component(_ArgTys&&... args)
			{
				auto constexpr category = _Ty::get_category();
				auto [iter, inserted]   = myComponents.try_emplace(category);

				if (inserted) 
				{
					auto  ptr = std::make_shared<_Ty>(std::forward<_ArgTys>(args)...);
					auto  me  = std::static_pointer_cast <Game::Object>(this->shared_from_this());

					ptr  -> set_owner (me);
					iter -> second = ptr;

					return ptr;
				}
				else {
					auto ptr = std::dynamic_pointer_cast<_Ty>(iter->second);

					if (ptr)
						return ptr;
					else
						x_another_incompatible_exists();
				}
			}

			template <GameComponent _Ty>
			_NODISCARD bool has_component() const noexcept 
			{
				auto constexpr category = _Ty::get_category();
				auto		   iter		= myComponents.find(category);

				if (iter != myComponents.end())
					return std::dynamic_pointer_cast<_Ty>(iter->second) != nullptr;
				else
					return false;
			}

			template <GameComponent _Ty>
			_NODISCARD std::weak_ptr <_Ty const> get_component() const
			{
				auto constexpr category = _Ty::get_category();
				auto		   iter     = myComponents.find(category);

				if (iter != myComponents.end())
				{
					auto ptr = std::dynamic_pointer_cast<_Ty>(iter->second);

					if (ptr)
						return ptr;
					else
						x_another_incompatible_exists();
				}
				else
					x_no_component();
			}

			template <GameComponent _Ty>
			_NODISCARD std::weak_ptr <_Ty> get_component() 
			{
				auto component = std::as_const(*this).get_component<_Ty>();
				return std::const_pointer_cast<_Ty>(component.lock());
			}

			template <GameComponent _Ty>
			void remove_component() noexcept {
				myComponents.erase(_Ty::get_category());
			}

		protected:
			void for_each (auto&& fn, auto&&... args) const {
				for (auto& [_, component] : myComponents)
					fn (*component, args...);
			}

		private:
			std::unordered_map <Game::ComponentBase::Category,
							    std::shared_ptr <Game::ComponentBase>>
			myComponents;
		};
	}

	namespace Game
	{
		class Scene;

		class Object :
			public Detail::AssetBase,
			public Detail::EntityBase,
			public Detail::LayeredBase,
			public Detail::ComponentsContainerBase
		{
			static void x_no_scene() {
				throw std::logic_error("Bad call in an object that doesn't have an scene");
			}

		public:
			Object() = default;

			Object(Object&&)	  = delete;
			Object(Object const&) = delete;

			Object& operator=(Object&&)		 = delete;
			Object& operator=(Object const&) = delete;

			void on_start() override {
				this->for_each([](ComponentBase& comp) { comp.start();});
			}

			void on_update(float time) override {
				this->for_each([=](ComponentBase& comp) { comp.update(time);});
			}

			void on_late_update(float time) override {
				this->for_each([=](ComponentBase& comp) { comp.late_update(time);});
			}

			void on_render() override {
				this->for_each([](ComponentBase& comp) { comp.render();});
			}

			void on_restore (nlohmann::json const& obj) override 
			{
				auto components = obj.find(Keys::components);

				if (components != obj.end() &&
					components->is_array()
				) {
					std::unordered_map <size_t, AssetBase*> assets;
					assets.reserve (components->size());

					this->for_each ([&](ComponentBase& comp) 
					{
						auto base  = &comp;
						auto asset = dynamic_cast<AssetBase*>(base);

						if (asset)
							assets.emplace (asset->get_id(), asset);
					});

					for (auto& component : *components)
					{
						if (component.is_object() &&
							component.contains (AssetBase::Keys::id)
						) {
							size_t id = component [AssetBase::Keys::id];

							if (assets.contains(id)) {
								assets[id]->restore(component);
								continue;
							}
						}
						
						x_cannot_restore();
					}
				}
				else
					AssetBase::x_cannot_restore();
			}
			
			_NODISCARD nlohmann::json on_save() const override 
			{
				std::vector <nlohmann::json> components;
				nlohmann::json				 object;

				this->for_each ([&](ComponentBase& comp) 
				{
					auto base  = &comp;
					auto asset = dynamic_cast<AssetBase*>(base);

					if (asset)
						components.push_back(asset->save());
				});

				auto& list = (object[Keys::components] = nlohmann::json::array());

				for (auto& comp : components)
					list.push_back (std::move(comp));

				return object;
			}

			_NODISCARD Scene const& get_scene() const {
				if (auto scene = myScene.lock())
					return *scene;
				else
					x_no_scene();
			}

			_NODISCARD Scene& get_scene() {
				return const_cast<Scene&>(std::as_const(*this).get_scene());
			}

		private:
			void set_scene(std::weak_ptr<Scene> scene) noexcept {
				myScene = scene;
			}

			friend class Detail::ObjectsContainerBase;

		private:
			std::weak_ptr <Scene> myScene;

			struct Keys {
				static constexpr std::string_view components = "componentsList";
			};
		};		
	}

	namespace Detail
	{
		template <class _Ty>
		concept GameObject = std::derived_from <_Ty, Game::Object>;
	}
}
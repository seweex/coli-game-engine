#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Object.hxx"

#include "../Visual/Camera.hxx"

#include "Components/PhysicalBody.hxx"
#include "Components/Collider.hxx"

namespace Coli
{
	namespace Generic
	{
		class Engine;
	}

	namespace Detail
	{
		class ObjectsContainerBase :
			public std::enable_shared_from_this <ObjectsContainerBase>
		{
		protected:
			ObjectsContainerBase() = default;

		public:
			ObjectsContainerBase(ObjectsContainerBase&&)	  = delete;
			ObjectsContainerBase(ObjectsContainerBase const&) = delete;

			ObjectsContainerBase& operator=(ObjectsContainerBase&&)	     = delete;
			ObjectsContainerBase& operator=(ObjectsContainerBase const&) = delete;

			template <GameObject _Ty, class ... _ArgTys>
				requires (std::constructible_from<_Ty, _ArgTys...>)
			_NODISCARD std::weak_ptr <_Ty> make_object(_ArgTys&&... args)
			{
				auto ptr = std::make_shared<_Ty>(std::forward<_ArgTys>(args)...);
				auto me  = std::static_pointer_cast <Game::Scene>(this->shared_from_this());

				ptr -> set_scene(me);
				myObjects.emplace(ptr);

				return ptr;
			}

			template <GameObject _Ty>
			void remove_object(std::shared_ptr<_Ty> const& ptr) noexcept {
				myObjects.erase(ptr);
			}

		private:
			void for_each(auto&& fn)
			{
				for (auto iter = myObjects.begin(); 
					 iter != myObjects.end();
				) {
					auto const& object = *iter;
					
					if (!object) throw 2;

					fn (*object);

					if (object->has_changed())
					{
						auto  node = myObjects.extract(iter++);
						auto& ptr  = node.value();

						myChangedObjects.emplace_back(std::move(ptr));
					}
					else
						++iter;
				}

				for (auto& changed : myChangedObjects)
					myObjects.emplace(std::move(changed));

				myChangedObjects.clear();
			}

		protected:
			void start_all() {
				for_each([] (Game::Object& obj) {
					obj.start();
				});
			}

			void update_all(float time) {
				for_each([=] (Game::Object& obj) {
					obj.update(time);
				});
			}

			void late_update_all(float time) {
				for_each([=] (Game::Object& obj) {
					obj.late_update(time);
				});
			}

			void render_all() {
				for_each([] (Game::Object& obj) {
					obj.render();
				});
			}

		private:
			using comparator_type = decltype([] (auto const& l, auto const& r) {
									    return l->get_layer() < r->get_layer();
									});

			using hasher_type = decltype([] (auto const& ptr) {
									return std::hash<size_t>{}(ptr->get_layer());
								});

			std::multiset <std::shared_ptr <Game::Object>, comparator_type> myObjects;
			std::vector   <std::shared_ptr <Game::Object>> myChangedObjects;
		};
	}

	namespace Game
	{
		class Scene final :
			public Detail::EntityBase,
			public Detail::ObjectsContainerBase
		{
		public:
			Scene (Generic::Engine& engine) noexcept :
				myEngine (engine)
			{}

			Scene(Scene&&)      = delete;
			Scene(Scene const&) = delete;

			Scene& operator=(Scene&&)	   = delete;
			Scene& operator=(Scene const&) = delete;

			void on_start() final {
				this->start_all();
			}

			void on_update(float time) final {
				this->update_all(time);
			}

			void on_late_update(float time) final {
				this->late_update_all(time);
			}

			void on_render() final {
				this->render_all();
			}

		private:
			Generic::Engine& myEngine;
		};
	}
}
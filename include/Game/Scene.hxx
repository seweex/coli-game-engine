#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Object.hxx"
#include "../Visual/Camera.hxx"

#include "Components/PhysicalBody.hxx"
#include "Components/Collider.hxx"

namespace Coli
{
	namespace Game
	{
		class Scene final
		{
			void sort_by_layers()
			{
				std::vector<std::shared_ptr<Object>> toReinsert;
				toReinsert.reserve(myObjects.size());

				for (auto iter = myObjects.begin();
					 iter     != myObjects.end();
				) {
					if ((*iter)->has_layer_changed()) _UNLIKELY
					{
						auto ptr = std::move(myObjects.extract(iter++).value());
						ptr->reset_layer_change();
						
						toReinsert.push_back(std::move(ptr));
					}
					else 
						++iter;
				}

				myObjects.insert(std::make_move_iterator(toReinsert.begin()),
								 std::make_move_iterator(toReinsert.end()));
			}

			void find_collisions()
			{
				std::vector <std::shared_ptr<Components::ColliderBase>> colliders;
				std::unordered_map <std::shared_ptr<Components::ColliderBase>,
								    std::shared_ptr<Components::PhysicalBodyBase>>
				recievers;
				
				recievers.reserve(myObjects.size());
				colliders.reserve(myObjects.size());

				for (auto const& obj : myObjects)
					if (auto colliderComponent = obj->try_get_component<Components::ColliderBase>().lock()) _UNLIKELY
					{
						if (auto physicalComponent = obj->try_get_component<Components::PhysicalBodyBase>().lock()) _UNLIKELY
							recievers.emplace(colliderComponent, physicalComponent);
						
						colliders.emplace_back(colliderComponent);
					}

				for (size_t i = 0; i < colliders.size(); ++i)
				{
					auto&      leftCollider    = colliders[i];
					auto	   leftPhysical    = recievers.find(leftCollider);
					bool const hasLeftPhysical = leftPhysical != recievers.end();

					for (size_t j = i + 1; j < colliders.size(); ++j)
					{
						auto&      rightCollider    = colliders[j];
						auto       rightPhysical    = recievers.find(rightCollider);
						bool const hasRightPhysical = rightPhysical != recievers.end();

						if (hasLeftPhysical && hasRightPhysical) _UNLIKELY
						{
							auto& thisPhysical  = (hasLeftPhysical ? leftPhysical : rightPhysical)->second;
							auto& otherPhysical = (hasLeftPhysical ? rightPhysical : leftPhysical)->second;
							
							if (auto collision = leftCollider->find_collision(*rightCollider)) _UNLIKELY
								thisPhysical->report_collision(*collision, *otherPhysical);
						}
						else if (hasLeftPhysical || hasRightPhysical) _LIKELY
						{
							auto& thisPhysical = (hasLeftPhysical ? leftPhysical : rightPhysical)->second;

							if (auto collision = leftCollider->find_collision(*rightCollider)) _UNLIKELY
								thisPhysical->report_collision(*collision);
						}
					}
				}
			}

		public:
			template <std::derived_from<Object> _Ty = Object, class..._ArgTys>
			_Ty& make_object(_ArgTys&&... args)
			{
				auto  ptr = std::make_shared<_Ty>(std::forward<_ArgTys>(args)...);
				auto& obj = *ptr;

				if constexpr (std::derived_from<_Ty, Detail::CameraBase>)
					if (myCamera.expired())
						myCamera = ptr;
				
				myObjects.emplace(std::move(ptr));
				return obj;
			}

			void bind_camera(std::shared_ptr<Detail::CameraBase> camera) noexcept {
				myCamera = camera;
			}

			_NODISCARD std::weak_ptr<Detail::CameraBase> get_active_camera() const noexcept {
				return myCamera;
			}

			void start_all() {
				for (auto& object : myObjects)
					object->correct_on_start();
			}

			void update_all(float time)
			{
				sort_by_layers();

				for (auto& object : myObjects)
					object->correct_on_update(time);

				for (auto& object : myObjects)
					object->correct_on_late_update(time);

				for (auto& object : myObjects)
					object->correct_on_render_update();

				find_collisions();
			}

			_NODISCARD nlohmann::json serialize() const 
			{
				auto obj = nlohmann::json::array();

				for (auto const& object : myObjects)
					obj.push_back(object->serialize());

				return obj;
			}

			void deserialize(nlohmann::json const obj) const
			{
				if (obj.is_array())
				{
					std::set<Object*, decltype([](auto const& l, auto const& r) {
												   return l->get_id() < r->get_id();
											   })> 
					idSortedObjects;
					
					std::transform(myObjects.begin (), 
								   myObjects.end   (),
								   std::inserter   (idSortedObjects, idSortedObjects.begin()),
								   [] (auto const& unique) {
								       return unique.get();
								   });

					for (auto const& objectData : obj)
					{
						size_t ID;
						Detail::Json::try_fill(objectData, ID, key_id);

						auto iter = std::lower_bound(idSortedObjects.begin(),
													 idSortedObjects.end(),
													 ID,
													 [] (auto const& ptr, auto const& id) {
													     return ptr->get_id() < id;
													 });

						if (iter != idSortedObjects.end() && (*iter)->get_id() == ID) _LIKELY
						{
							(*iter)->deserialize(objectData);
							idSortedObjects.erase(iter);
						}						
					}
				}
				else
					throw std::invalid_argument("invalid json object: it's not an array");
			}

		private:
			static constexpr std::string_view key_id = "id";

			std::weak_ptr<Detail::CameraBase> myCamera;

			std::multiset<std::shared_ptr<Object>, 
						  decltype([](auto const& l, auto const& r) {
					          return l->get_layer() < r->get_layer();
						  })>
			myObjects;
		};
	}
}
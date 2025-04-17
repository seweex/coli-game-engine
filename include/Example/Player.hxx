#pragma once

#include "Blocks.hxx"

#include "../Game/Object.hxx"
#include "../Game/Component.hxx"

namespace Blocks
{
	namespace Detail
	{
		struct AxesNames final
		{
			static constexpr std::string_view forward = "PlayerForward";
			static constexpr std::string_view sideway = "PlayerSideway";
			static constexpr std::string_view fly     = "PlayerFly";

			static constexpr std::string_view pitch = "PlayerPitch";
			static constexpr std::string_view yaw   = "PlayerYaw";
		};
	}

	namespace Game
	{
		namespace Components
		{ 
			class UserControlScript final :
				public Coli::Game::ScriptBase
			{ 
			public:
				UserControlScript(Coli::Input::Map& inputMap) :
					myInputMap (inputMap)
				{}

				void take_dependencies() final 
				{
					auto& owner = get_owner();

					myTransform    = owner.try_get_component<Coli::Game::Components::Transform<float>>();
					myPhysicalBody = owner.try_get_component<Coli::Game::Components::PhysicalBody<float>>();
				}

				void on_update(float time) final
				{ 
					auto const physicalBody = myPhysicalBody.lock();
					auto const transform    = myTransform.lock();

					if (physicalBody && transform) _LIKELY
					{
						constexpr float min_pitch = glm::radians(-89.f);
						constexpr float max_pitch = glm::radians( 89.f);

						float const yaw   = myInputMap.get_axis_value(Detail::AxesNames::yaw);
						float const pitch = glm::clamp<float>(myInputMap.get_axis_value(Detail::AxesNames::pitch), min_pitch, max_pitch); 

						glm::quat const rotation = glm::angleAxis (pitch, glm::vec3{ 0, -1, 0 })
												 * glm::angleAxis (yaw,   glm::vec3{ -1, 0, 0 });

						glm::vec3 const inputs {
							myInputMap.get_axis_value (Detail::AxesNames::sideway),
							myInputMap.get_axis_value (Detail::AxesNames::fly),
							myInputMap.get_axis_value (Detail::AxesNames::forward)
						};

						if (glm::length2(inputs) > 0) _UNLIKELY
							physicalBody->report_force(inputs, time);

						transform->rotation = rotation;
					}
					else
						take_dependencies();
				}

			private:
				Coli::Input::Map& myInputMap;

				std::weak_ptr<Coli::Game::Components::Transform<float>>    myTransform;
				std::weak_ptr<Coli::Game::Components::PhysicalBody<float>> myPhysicalBody;
			};

			class FollowObjectScript final :
				public Coli::Game::ScriptBase
			{
			public:
				FollowObjectScript (std::weak_ptr<Coli::Game::Object> followed) :
					myFollowed (followed)
				{
					if (followed.expired())
						throw std::invalid_argument("pass a valid object");
				}

				void take_dependencies() final
				{
					if (auto followed = myFollowed.lock()) _LIKELY {
						myTransform         = get_owner().try_get_component<Coli::Game::Components::Transform<float>>();
						myFollowedTransform = followed->try_get_component<Coli::Game::Components::Transform<float>>();
					}					
				}

				void on_update(float time) final
				{
					std::shared_ptr<Coli::Geometry::Transform<float>> const		  transform		    = myTransform.lock();
					std::shared_ptr<Coli::Geometry::Transform<float> const> const followedTransform = myFollowedTransform.lock();

					if (transform && followedTransform) _LIKELY
						*transform = *followedTransform;
					
					else if (!myFollowed.expired())
						take_dependencies();
				}

			private:
				std::weak_ptr <Coli::Game::Object> myFollowed;

				std::weak_ptr <Coli::Game::Components::Transform<float>>       myTransform;
				std::weak_ptr <Coli::Game::Components::Transform<float> const> myFollowedTransform;
			};
		}

		namespace Objects
		{
			class Player final :
				public Coli::Game::Object
			{
			public:
				Player (Coli::Input::Map& inputMap) 
				{
					make_component <Coli::Game::Components::Transform<float>>();
					make_component <Coli::Game::Components::PhysicalBody<float>>();
					
					make_component <Game::Components::UserControlScript>(inputMap);
				}			
			};
		}
	}
}
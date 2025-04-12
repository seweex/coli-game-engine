#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include <Game/Object.hxx>
#include <Game/objects/Camera.hxx>
#include <Game/Components/Transform.hxx>
#include <Game/Components/PhysicalBody.hxx>

#include <Input/Map.hxx>

#include <Generic/Engine.hxx>

#include <iostream>

namespace Blocks
{
	namespace Game
	{
		namespace Components
		{
			class MovementControl :
				public Coli::Game::ScriptBase
			{
			public:
				MovementControl (Coli::Input::Map& map) noexcept :
					myInputMap (map)
				{}

				void take_dependencies() final 
				{
					auto& owner = get_owner();

					myPhysicalBody = owner.try_get_component<Coli::Game::Components::PhysicalBody<float>>();
					myTransform    = owner.try_get_component<Coli::Game::Components::Transform<float>>();
				}

				void on_update(float time) final
				{
					auto const physicalBody = myPhysicalBody.lock();
					auto const transform    = myTransform.lock();

					if (physicalBody && transform) _LIKELY
					{
					    float const yaw   = myInputMap.get_axis_value(yaw_axis);
						float       pitch = myInputMap.get_axis_value(pitch_axis);

						pitch = glm::clamp(pitch, glm::radians(-89.f), glm::radians(89.f));

						glm::quat const rotation = glm::angleAxis(pitch, glm::vec3{ 0, -1, 0 })
												 * glm::angleAxis(yaw,   glm::vec3{ -1, 0, 0 });

						glm::vec3 const inputs {
							myInputMap.get_axis_value(sideway_axis),
							myInputMap.get_axis_value(fly_axis),
							myInputMap.get_axis_value(forward_axis)
						};

						if (glm::length2(inputs) > 0) _UNLIKELY
							physicalBody->report_force(inputs, speed);

						transform->rotation = rotation;
					}
				}

				static constexpr std::string_view forward_axis = "PlayerForward";
				static constexpr std::string_view sideway_axis = "PlayerSideway";
				static constexpr std::string_view fly_axis     = "PlayerFly";

				static constexpr std::string_view pitch_axis = "PlayerPitch";
				static constexpr std::string_view yaw_axis   = "PlayerYaw";

			private:
				static constexpr float speed = 0.15;

				std::weak_ptr <Coli::Game::Components::PhysicalBody<float>> myPhysicalBody;
				std::weak_ptr <Coli::Game::Components::Transform<float>>    myTransform;

				Coli::Input::Map& myInputMap;
			};

			class SameTransformScript :
				public Coli::Game::ScriptBase
			{
			public:
				SameTransformScript(std::weak_ptr<Coli::Game::Object const> follow) :
					myFollowed (follow)
				{}

				void take_dependencies() final 
				{
					myTransform = get_owner().try_get_component<Coli::Game::Components::Transform<float>>();

					if (auto other = myFollowed.lock())
						myFollowedTransform = other->try_get_component<Coli::Game::Components::Transform<float>>();
				}

				void on_update(float time) noexcept final
				{
					using Coli::Geometry::Transform;

					std::shared_ptr <Transform<float>>	     transform      = myTransform.lock();
					std::shared_ptr <Transform<float> const> otherTransform = myFollowedTransform.lock();

					if (transform && otherTransform) _LIKELY
						*transform = *otherTransform;
				}

			private:
				std::weak_ptr <Coli::Game::Object const> myFollowed;

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
				Player (Coli::Input::Map& map) :
					myTransform (this->make_component<Coli::Game::Components::Transform<float>>())
				{
					this->make_component<Components::MovementControl>(map);
				}

			private:
				std::weak_ptr <Coli::Game::Components::Transform<float>> myTransform;
			};
		}
	}

	static int go()
	{
		using namespace Coli;

		try {
			constexpr std::string_view application_name = "Blocks: Coli Game Engine test";
			constexpr std::string_view main_scene_name  = "World";

			Generic::Engine engine { application_name };
			
			auto& inputMap = engine.get_input_manager();
			auto& scene    = engine.make_scene();

			{
				using Control = Blocks::Game::Components::MovementControl;
				constexpr double sensitivity = 0.00175;

				inputMap.make_axis(Control::forward_axis, GLFW_KEY_S, GLFW_KEY_W);
				inputMap.make_axis(Control::sideway_axis, GLFW_KEY_D, GLFW_KEY_A);
				inputMap.make_axis(Control::fly_axis,     GLFW_KEY_SPACE, GLFW_KEY_LEFT_CONTROL);

				inputMap.make_axis(Control::pitch_axis, sensitivity, Detail::MouseAxisDependence::cursor_y);
				inputMap.make_axis(Control::yaw_axis,   sensitivity, Detail::MouseAxisDependence::cursor_x);
			}

			{
				namespace BGame = Blocks::Game;
				namespace CGame = Coli::Game;

				auto player = scene.make_object<BGame::Objects::Player>(inputMap).lock();
				auto camera = scene.make_object<CGame::Objects::Camera<float>>().lock();

				camera->make_component<BGame::Components::SameTransformScript>(player);
			}

			try {
				engine.load_scene(main_scene_name);
				std::cout << "Loaded scene '" << main_scene_name << "'" << std::endl;
			}
			catch (std::bad_alloc const& exc) {
				throw;
			}
			catch (std::exception const& exc) 
			{
				std::cerr << "Failed to load the scene. Default values are accepted.\n"
						  << "Error description: "
						  << exc.what() << std::endl;
			}

			engine.run();

			engine.save_scene    (main_scene_name);
			engine.save_settings ();
		}
		catch (std::bad_alloc const& exc) 
		{
			std::cerr << "Allocation failed" << std::endl;
			return -1;
		}
	}
}

int main() {
	return Blocks::go();
}
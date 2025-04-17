#pragma once

#include "Player.hxx"

#include "../Generic/Engine.hxx"

namespace Blocks
{
	namespace Detail
	{
		template <class _Ty>
		class SingletonBase 
		{
		protected:
			SingletonBase() {
				if (std::exchange(exists, true))
					throw std::runtime_error("the singleton already exists");
			}

		public:
			~SingletonBase() {
				exists = false;
			}

		private:
			static inline bool exists = false;
		};
	}

	class GameInstance final :
		private Detail::SingletonBase <GameInstance>
	{
	public:
		GameInstance() :
			myEngine (application_name)
		{
			auto& inputMap = myEngine.get_input_manager();
			auto& scene    = myEngine.make_scene();

			{
				constexpr double sensitivity = 0.00175;

				inputMap.make_axis(Detail::AxesNames::forward, GLFW_KEY_S, GLFW_KEY_W);
				inputMap.make_axis(Detail::AxesNames::sideway, GLFW_KEY_D, GLFW_KEY_A);
				inputMap.make_axis(Detail::AxesNames::fly,     GLFW_KEY_SPACE, GLFW_KEY_LEFT_CONTROL);

				inputMap.make_axis(Detail::AxesNames::pitch, sensitivity, Coli::Detail::MouseAxisDependence::cursor_y);
				inputMap.make_axis(Detail::AxesNames::yaw,   sensitivity, Coli::Detail::MouseAxisDependence::cursor_x);
			}

			{
				auto const player = scene.make_object<Game::Objects::Player>(inputMap).lock();
				auto const camera = scene.make_object<Coli::Game::Objects::Camera<float>>().lock();

				camera->make_component<Game::Components::FollowObjectScript>(player);
			}
		}

		void run() {
			myEngine.run();
		}

		bool load() {
			try {
				myEngine.load_scene(main_scene_name);
				std::cout << "Loaded scene '" << main_scene_name << "'" << std::endl;
				return true;
			}
			catch (std::bad_alloc const&) { throw; }
			catch (std::exception const& exc)
			{
				std::cerr << "Failed to load the scene. Default values are accepted.\n"
						  << "Error description: "
						  << exc.what() << std::endl;

				return false;
			}
		}

		void save() const {
			myEngine.save_scene(main_scene_name);
			myEngine.save_settings();
		}

	private:
		static constexpr std::string_view application_name = "Blocks - Coli-Game-Engine test";
		static constexpr std::string_view main_scene_name  = "World";

		Coli::Generic::Engine myEngine;
	};
}
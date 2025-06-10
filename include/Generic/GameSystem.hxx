#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "../Game/Scene.hxx"

namespace Coli
{
	namespace Generic
	{
		class GameSystem final 
		{
			static void x_alien_scene() {
				throw std::invalid_argument("An alien scene cannot be active here");
			}

		public:
			GameSystem (Generic::Engine& engine) :
				myEngine (engine)
			{}

			GameSystem(GameSystem&&)	  = delete;
			GameSystem(GameSystem const&) = delete;

			GameSystem& operator=(GameSystem&&)		 = delete;
			GameSystem& operator=(GameSystem const&) = delete;

			_NODISCARD std::shared_ptr <Game::Scene> make_scene() 
			{
				auto ptr = std::make_shared <Game::Scene>(myEngine);

				if (myActiveScene.expired())
					myActiveScene = ptr;

				return ptr;
			}

			void remove_scene(std::weak_ptr <Game::Scene> scene) noexcept 
			{
				auto ptr    = scene.lock();
				auto active = myActiveScene.lock();

				if (ptr == active)
					myActiveScene.reset();

				myScenes.erase(ptr);
			}

			void make_scene_active(std::weak_ptr <Game::Scene> scene)
			{
				auto ptr = scene.lock();

				if (myScenes.contains(ptr))
					myActiveScene = scene;
				else
					x_alien_scene();
			}

			void reset_active_scene() noexcept {
				myActiveScene.reset();
			}

			_NODISCARD std::weak_ptr <Game::Scene const> get_active_scene() const noexcept {
				return myActiveScene;
			}

			_NODISCARD std::weak_ptr <Game::Scene> get_active_scene() noexcept {
				return myActiveScene;
			}

		private:
			friend class Engine;

			void start() {
				if (auto scene = myActiveScene.lock())
					scene->start();
			}

			void update(float time) {
				if (auto scene = myActiveScene.lock())
					scene->update(time);
			}

			void late_update(float time) {
				if (auto scene = myActiveScene.lock())
					scene->late_update(time);
			}

			void render() {
				if (auto scene = myActiveScene.lock())
					scene->render();
			}

		private:
			Engine& myEngine;

			std::unordered_set <std::shared_ptr <Game::Scene>> myScenes;
			std::weak_ptr <Game::Scene>						   myActiveScene;
		};
	}
}
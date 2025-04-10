#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Module.hxx"

#include "../Input/Map.hxx"
#include "../File/Loader.hxx"
#include "../Game/Scene.hxx"

#include "../Graphics/Window.hxx"

namespace Coli
{
	namespace Detail
	{
		class TimeManagerBase
		{
		protected:
			TimeManagerBase() noexcept :
				myCurrentTime  (std::chrono::system_clock::now()),
				myPreviousTime (myCurrentTime)
			{}

			void update_time() noexcept {
				myPreviousTime = myCurrentTime;
				myCurrentTime  = std::chrono::system_clock::now();
			}

		public:
			_NODISCARD float get_delta_time() const noexcept {
				return std::chrono::duration_cast<std::chrono::duration<float>>(myCurrentTime - myPreviousTime).count();
			}

		private:
			std::chrono::system_clock::time_point myCurrentTime;
			std::chrono::system_clock::time_point myPreviousTime;
		};
	}

	namespace Generic
	{
		class Engine final :
			public Detail::TimeManagerBase
		{
			_NODISCARD std::string make_config_name() const 
			{
				using namespace std::string_literals;

				return myApplicationName + ".config"s;
			}

			_NODISCARD std::string make_scene_name(std::string_view name) const
			{
				using namespace std::string_literals;

				return name.data() + ".scene"s;
			}

		public:
			Engine (std::string_view applicationName) :
				myApplicationName (applicationName),
				myInputMap		  (std::make_unique<Input::Map>()),
				myFileLoader	  (std::make_unique<File::Loader>()),
				myShutdownFlag    (false)
			{
				using namespace std::string_literals;

				auto const settings = myFileLoader->load_settings(make_config_name());

				myWindow.emplace(*myInputMap, applicationName, settings.windowWidth, settings.windowHeight);
			}

			_NODISCARD Input::Map& get_input_manager() noexcept {
				return *myInputMap;
			}

			_NODISCARD File::Loader& get_file_manager() noexcept {
				return *myFileLoader;
			}

			_NODISCARD std::weak_ptr<Game::Scene> get_active_scene() noexcept {
				return myScene;
			}

			std::weak_ptr<Game::Scene> make_scene() {
				return myScene = std::make_shared<Game::Scene>();
			}

			void set_active_scene(std::shared_ptr<Game::Scene> scene) noexcept {
				myScene.swap(scene);
			}

			void save_settings() const
			{
				auto const [windowWidth, windowHeight] = myWindow->get_sizes();

				Detail::Settings const settings   { windowWidth, windowHeight };
				auto const			   configName = make_config_name();

				myFileLoader->save_settings(make_config_name(), settings);
			}

			void save_scene(std::string_view name) const
			{
				if (myScene) _LIKELY
				{
					auto const serialized = myScene->serialize();
					auto const sceneName  = make_scene_name(name);
					
					myFileLoader->save_serialized(sceneName, serialized);
				}
			}

			Game::Scene& load_scene(std::string_view name, Game::Scene& out) const
			{
				auto const deserialized = myFileLoader->load_serialized(make_scene_name(name));
				
				out.deserialize(deserialized);
				return out;
			}

			Game::Scene& load_scene(std::string_view name) {
				if (myScene)
					return load_scene(name, *myScene);
				else
					throw std::runtime_error("provide a scene for deserializing");
			}

			void shutdown() noexcept {
				myShutdownFlag = true;
			}

			void run()
			{
				if (myScene) _LIKELY
				{
					std::weak_ptr<Graphics::Renderer> activeRenderer;
					std::weak_ptr<Detail::CameraBase> activeCamera;

					myWindow->show();
					myScene->start_all();

					while (!myShutdownFlag)
					{
						if (activeRenderer.expired()) _UNLIKELY
							activeRenderer = Detail::DrawableBase::get_renderer();

						if (activeCamera.expired()) _UNLIKELY 
						{
							activeCamera = myScene->get_active_camera();
						    
							if (auto lockedCamera = activeCamera.lock()) _LIKELY {
								auto const [windowWidth, windowHeight] = myWindow->get_sizes();
								lockedCamera->update_aspect(windowWidth / static_cast<float>(windowHeight));
							}
						}

						{
							auto renderer = activeRenderer.lock();
							auto camera   = activeCamera.lock();

							if (renderer && camera) _LIKELY
								renderer->update(*camera);
						}

						auto const deltaTime = glm::max(get_delta_time(), std::numeric_limits<float>::epsilon());

						myWindow-> update();
						myScene->  update_all(deltaTime);
						this->     update_time();

						myShutdownFlag |= myWindow->should_close();
					}
				}
				else
					throw std::runtime_error("no active scene set");
			}

		private:
			std::string myApplicationName;

			std::unique_ptr <Input::Map> const   myInputMap;
			std::unique_ptr <File::Loader> const myFileLoader;
			std::shared_ptr <Game::Scene>        myScene;

			Graphics::Context				myContext;
			std::optional<Graphics::Window> myWindow;

			bool myShutdownFlag;
		};
	}
}
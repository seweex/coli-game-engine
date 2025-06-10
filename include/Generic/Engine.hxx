#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Configuration.hxx"
#include "TimeManager.hxx"
#include "FileSystem.hxx"
#include "GameSystem.hxx"
#include "GraphicSystem.hxx"

namespace Coli
{
	namespace Generic
	{
		class Engine final
		{
			_NODISCARD std::string make_data_path() const {
				return "./" + myApplicationName + "/data/";
			}

			void load() {
				myInputSystem = std::make_shared <Input::Map> ();
				myFileSystem  = std::make_unique <FileSystem> (make_data_path());
				myGameSystem  = std::make_unique <GameSystem> (*this);

				auto const configuration = myFileSystem->load_config();

				myGraphicSystem = std::make_unique <GraphicSystem>(configuration.windowConfig);

				auto& window = myGraphicSystem->get_window();
				window.bind_input_map(myInputSystem);
			}

		public:
			Engine(std::string_view applicationName) :
				myApplicationName (applicationName)
			{
				load();
			}

			Engine(Engine&&)      = delete;
			Engine(Engine const&) = delete;

			Engine& operator=(Engine&&)      = delete;
			Engine& operator=(Engine const&) = delete;

			_NODISCARD TimeManager const& get_time_manager() const noexcept {
				return myTimeManager;
			}

			_NODISCARD GameSystem const& get_game_system() const noexcept {
				return *myGameSystem;
			}

			_NODISCARD GameSystem& get_game_system() noexcept {
				return *myGameSystem;
			}
			
			_NODISCARD FileSystem const& get_file_system() const noexcept {
				return *myFileSystem;
			}

			_NODISCARD FileSystem& get_file_system() noexcept {
				return *myFileSystem;
			}

			_NODISCARD Input::Map const& get_input_system() const noexcept {
				return *myInputSystem;
			}

			_NODISCARD Input::Map& get_input_system() noexcept {
				return *myInputSystem;
			}

			void stop() {
				myRunningFlag = false;
			}

			void run()
			{
				auto& gameSystem = *myGameSystem;
				auto& window     = myGraphicSystem->get_window();

				while (myRunningFlag)
				{
					auto deltaTime = myTimeManager.get_delta_time();

					gameSystem.update(deltaTime);
					gameSystem.late_update(deltaTime);
					gameSystem.render();

					myTimeManager.update();
					window.update();

					myRunningFlag = myRunningFlag && !window.should_close();
				}
			}

			void save() {
				Configuration cfg;

				auto& window = myGraphicSystem->get_window();
				auto const [windowWidth, windowHeight] = window.size();

				cfg.windowConfig.width = windowWidth;
				cfg.windowConfig.height = windowHeight;
				cfg.windowConfig.title = myApplicationName;

				myFileSystem->save_config(cfg);
			}

		private:
			/* necessary systems */
			TimeManager						myTimeManager;
			std::unique_ptr <GraphicSystem> myGraphicSystem;
			std::unique_ptr <GameSystem>	myGameSystem;
			std::unique_ptr <FileSystem>    myFileSystem;
			std::shared_ptr <Input::Map>    myInputSystem;

			/* lazy initialized (optional) systems */
			// ...

			std::string myApplicationName;
			bool		myRunningFlag;
		};
	}
}
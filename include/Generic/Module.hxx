#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "../Input/Map.hxx"
#include "../File/Loader.hxx"
#include "../Game/Scene.hxx"

#include "../Graphics/Window.hxx"

namespace Coli
{
	namespace Generic
	{
		class ModuleBase
		{
		public:
			virtual void on_update(float time) = 0;
		};

		class StatsTrackerModule final :
			public ModuleBase
		{
		public:
			void on_update(float time) final
			{
				myTimeAccumulator += time;
				++myFramesCount;

				if (time > myMaxDeltaTime)
					myMaxDeltaTime = time;

				if (myTimeAccumulator >= myOutputPeriod)
				{
					std::cout <<
						"Stats tracker info:\n" <<
						"Average FPS: " << myFramesCount / myTimeAccumulator << "\n"
						"Max delta time: " << myMaxDeltaTime << "\n" << 
						std::endl;

					myFramesCount     = 0;
					myTimeAccumulator = 0;
					myMaxDeltaTime    = 0;
				}					
			}

		private:
			size_t myFramesCount = 0;

			float		myMaxDeltaTime = 0;
			float		myTimeAccumulator = 0;
			float const myOutputPeriod    = 5;
		};
	}
}
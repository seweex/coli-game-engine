#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

namespace Coli
{
	namespace Generic
	{
		class TimeManager final
		{
		public:
			TimeManager() noexcept :
				myStartTime    (std::chrono::system_clock::now()),
				myCurrentTime  (myStartTime),
				myPreviousTime (myStartTime)
			{}

			TimeManager(TimeManager&&)	    = default;
			TimeManager(TimeManager const&) = default;

			TimeManager& operator=(TimeManager&&)	   = default;
			TimeManager& operator=(TimeManager const&) = default;

			_NODISCARD float get_delta_time() const noexcept 
			{
				auto difference = myCurrentTime - myPreviousTime;
				auto duration   = std::chrono::duration_cast<std::chrono::duration<float>>(difference);

				return duration.count();
			}

			_NODISCARD float get_time() const noexcept 
			{
				auto currentTime = std::chrono::system_clock::now();
				auto duration    = std::chrono::duration_cast<std::chrono::duration<float>>(currentTime - myStartTime);

				return duration.count();
			}

			void update() noexcept {
				myPreviousTime = myCurrentTime;
				myCurrentTime  = std::chrono::system_clock::now();
			}

		private:
			std::chrono::system_clock::time_point myStartTime;

			std::chrono::system_clock::time_point myCurrentTime;
			std::chrono::system_clock::time_point myPreviousTime;
		};
	}
}
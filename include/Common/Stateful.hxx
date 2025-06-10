#pragma once

#include "../Common.hxx"

namespace Coli
{
	namespace Detail
	{
		class StatefulBase
		{
		protected:
			StatefulBase() noexcept = default;

		public:
			StatefulBase (StatefulBase const& other) noexcept :
				isStarted (false),
				isVisible (other.isVisible),
				isActive  (other.isActive)
			{}

			StatefulBase& operator=(StatefulBase const& other) noexcept 
			{
				isVisible = other.isVisible;
				isActive  = other.isActive;

				return *this;
			}

			StatefulBase(StatefulBase&&) noexcept		     = default;
			StatefulBase& operator=(StatefulBase&&) noexcept = default;

			_NODISCARD bool is_started() const noexcept {
				return isStarted;
			}

			_NODISCARD bool is_visible() const noexcept {
				return isVisible;
			}

			_NODISCARD bool is_active() const noexcept {
				return isActive;
			}

			void enable() noexcept {
				isActive = true;
			}

			void disable() noexcept {
				isActive = false;
			}

			void show() noexcept {
				isVisible = false;
			}

			void hide() noexcept {
				isVisible = false;
			}

		protected:
			void mark_started() noexcept {
				isStarted = true;
			}

		private:
			bool isStarted = false;
			bool isVisible = true;
			bool isActive  = true;
		};
	}
}
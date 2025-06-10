#pragma once

#include "../Common.hxx"
#include "../Common/Stateful.hxx"

namespace Coli
{
	namespace Detail
	{
		class BehavioralBase :
			public virtual StatefulBase
		{
		protected:
			BehavioralBase() noexcept = default;

		public:
			virtual ~BehavioralBase() noexcept = default;

			BehavioralBase(BehavioralBase &&) noexcept	   = default;
			BehavioralBase(BehavioralBase const&) noexcept = default;

			BehavioralBase& operator=(BehavioralBase &&) noexcept	  = default;
			BehavioralBase& operator=(BehavioralBase const&) noexcept = default;

			virtual void on_start() {}
			virtual void on_update(float time) {}
			virtual void on_late_update(float time) {}
			virtual void on_render() {}

			virtual void start() {
				if (this->is_active() && !this->is_started()) {
					on_start();
					this->mark_started();
				}
			}

			virtual void update(float time) {
				if (this->is_active()) {
					if (!this->is_started())
						on_start();

					on_update(time);
				}
			}

			virtual void late_update(float time) {
				if (this->is_active()) {
					if (!this->is_started())
						on_start();

					on_late_update(time);
				}
			}

			virtual void render() {
				if (this->is_active() && this->is_visible()) {
					if (!this->is_started())
						on_start();

					on_render();
				}
			}
		};
	}
}
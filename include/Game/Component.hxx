#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

namespace Coli
{
	namespace Game
	{
		class ComponentBase;
	}

	namespace Detail
	{
		enum class ComponentCategory {
			transform, 
			collider, 
			physical_body,
			drawable, 
			tag,
			script
		};

		template <class _Ty>
		concept GameComponent = requires (_Ty _val)
		{
			{ _Ty::get_category() } -> std::same_as <ComponentCategory>;

			requires std::derived_from<_Ty, Game::ComponentBase>;
		};

		class EntityBase :
			public Detail::IdentifiableBase
		{
		public:
			_NODISCARD virtual nlohmann::json serialize() const = 0;
			virtual void deserialize(nlohmann::json const& obj) = 0;

			virtual void on_start () {}

			virtual void on_update        (float time) {}
			virtual void on_late_update   (float time) {}
			virtual void on_render_update () {}

			virtual void correct_on_start() {
				if (isActive && !hasStarted) _LIKELY {
					on_start();
					hasStarted = true;
				}
			}

			virtual void correct_on_update(float time) {
				if (isActive) _LIKELY
				{
					if (!hasStarted) _UNLIKELY {
						on_start();
						hasStarted = true;
					}
					else
						on_update(time);
				}
			}

			virtual void correct_on_late_update(float time) {
				if (isActive) _LIKELY
					on_late_update(time);
			}

			virtual void correct_on_render_update() {
				if (isActive) _LIKELY
					on_render_update();
			}
			
			_NODISCARD bool is_active() const noexcept {
				return isActive;
			}

			void enable() noexcept {
				isActive = true;
			}

			void disable() noexcept {
				isActive = true;
			}

		private:
			bool isActive   = true;
			bool hasStarted = false;
		};
	}

	namespace Game
	{
		class Object;

		class ComponentBase :
			public Detail::EntityBase
		{
		protected:
			ComponentBase() noexcept = default;

		public:
			_NODISCARD Object& get_owner() {
				if (myOwner) _LIKELY
					return *myOwner;
				else
					throw std::runtime_error("owner hasn't bound");
			}

			_NODISCARD Object const& get_owner() const {
				if (myOwner) _LIKELY
					return *myOwner;
				else
					throw std::runtime_error("owner hasn't bound");
			}

			void set_owner(Object& newOwner) noexcept {
				myOwner = std::addressof(newOwner);
			}

			virtual void take_dependencies() {}

		private:
			Object* myOwner = nullptr;
		};

		class ScriptBase :
			public ComponentBase
		{
		protected:
			ScriptBase() noexcept = default;

		public:
			_NODISCARD static constexpr Detail::ComponentCategory get_category() noexcept {
				return Detail::ComponentCategory::script;
			}

			_NODISCARD nlohmann::json serialize() const noexcept final { return {}; }
			void deserialize(nlohmann::json const&)     noexcept final {}
		};
	}
}
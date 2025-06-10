#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Asset.hxx"
#include "Entity.hxx"

namespace Coli
{
	namespace Detail
	{
		class ComponentsContainerBase;
	}

	namespace Game
	{
		class Object;

		class ComponentBase :
			public Detail::EntityBase
		{
			static void x_no_owner() {
				throw std::runtime_error ("Bad call in an object that doesn't have an owner");
			}

		protected:
			ComponentBase() noexcept = default;

		public:
			static constexpr size_t categories_count = 5;

			enum class Category {
				transform,
				collider,
				physical_body,
				drawable,
				script
			};

			ComponentBase(ComponentBase&&)		= delete;
			ComponentBase(ComponentBase const&) = delete;

			ComponentBase& operator=(ComponentBase&&)	   = delete;
			ComponentBase& operator=(ComponentBase const&) = delete;

			_NODISCARD Object const& get_owner() const 
			{
				if (auto ptr = myOwner.lock())
					return *ptr;
				else
					x_no_owner();
			}

			_NODISCARD Object& get_owner() {
				return const_cast<Object&>(std::as_const(*this).get_owner());
			}

		private:
			void set_owner(std::weak_ptr <Object> newOwner) noexcept {
				myOwner = newOwner;
			}

			friend class Detail::ComponentsContainerBase;

		private:
			std::weak_ptr <Object> myOwner;
		};

		class ScriptBase :
			public ComponentBase
		{
		protected:
			ScriptBase() noexcept = default;

		public:
			ScriptBase(ScriptBase&&)	  = delete;
			ScriptBase(ScriptBase const&) = delete;

			ScriptBase& operator=(ScriptBase&&)		 = delete;
			ScriptBase& operator=(ScriptBase const&) = delete;

			_NODISCARD static constexpr Category get_category() noexcept {
				return Category::script;
			}
		};
	}

	namespace Detail
	{
		template <class _Ty>
		concept GameComponent = requires (_Ty _val)
		{
			{ _Ty::get_category() } -> std::same_as <Game::ComponentBase::Category>;

			requires std::derived_from<_Ty, Game::ComponentBase>;
		};
	}
}
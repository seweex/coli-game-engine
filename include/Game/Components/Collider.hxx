#pragma once

#include "../../Common.hxx"
#include "../../Utility.hxx"

#include "Transform.hxx"
#include "../Component.hxx"
#include "../../Geometry/Collider.hxx"

namespace Coli
{
	namespace Game
	{
		namespace Components
		{
			class ColliderBase :
				public Detail::AssetBase,
				public ComponentBase
			{
			protected:
				ColliderBase() noexcept = default;

			public:
				void start()  noexcept final {}
				void render() noexcept final {}

				void update (float)		 noexcept final {}
				void late_update (float) noexcept final {}

				_NODISCARD static constexpr ComponentBase::Category get_category() noexcept {
					return ComponentBase::Category::collider;
				}
			};

			template <bool _Use2D>
			class BasicBoxCollider final :
				public ColliderBase,
				public Geometry::BasicBoxCollider <_Use2D>
			{
				using vector_type  = glm::vec <_Use2D ? 2 : 3, double>;
				using rotator_type = std::conditional_t <_Use2D, double, glm::dquat>;

			public:
				BasicBoxCollider (
					vector_type const& size,
					rotator_type const& rotator = Detail::default_rotator <rotator_type>
				) noexcept :
					Geometry::BasicBoxCollider<_Use2D>(size, rotator)
				{}

				BasicBoxCollider(BasicBoxCollider&&)	  = delete;
				BasicBoxCollider(BasicBoxCollider const&) = delete;

				BasicBoxCollider& operator=(BasicBoxCollider&&)		 = delete;
				BasicBoxCollider& operator=(BasicBoxCollider const&) = delete;

				void on_restore (nlohmann::json const& obj) final 
				{
					auto& base     = static_cast <Geometry::BasicBoxCollider <_Use2D>&>(*this);
					auto  restored = static_cast <Geometry::BasicBoxCollider <_Use2D>>(obj);
					
					base = restored;
				}
			
				_NODISCARD nlohmann::json on_save() const final
				{
					auto const&    base = static_cast <Geometry::BasicBoxCollider <_Use2D> const&>(*this);
					nlohmann::json object;

					object = base;
					return object;
				}
			};

			template <bool _Use2D>
			class BasicRoundCollider final :
				public ColliderBase,
				public Geometry::BasicRoundCollider <_Use2D>
			{
			public:
				BasicRoundCollider (double radius) noexcept :
					Geometry::BasicRoundCollider<_Use2D>(radius)
				{}
					
				BasicRoundCollider(BasicRoundCollider&&)	  = delete;
				BasicRoundCollider(BasicRoundCollider const&) = delete;

				BasicRoundCollider& operator=(BasicRoundCollider&&)		 = delete;
				BasicRoundCollider& operator=(BasicRoundCollider const&) = delete;

				void on_restore (nlohmann::json const& obj) final 
				{
					auto& base     = static_cast <Geometry::BasicRoundCollider <_Use2D>&>(*this);
					auto  restored = static_cast <Geometry::BasicRoundCollider <_Use2D>>(obj);
					
					base = restored;
				}
			
				_NODISCARD nlohmann::json on_save() const final
				{
					auto const&    base = static_cast <Geometry::BasicRoundCollider <_Use2D> const&>(*this);
					nlohmann::json object;

					object = base;
					return object;
				}
			};

			using BoxCollider   = BasicBoxCollider <false>;
			using BoxCollider2D = BasicBoxCollider <true>;

			using SphereCollider   = BasicRoundCollider <false>;
			using CircleCollider2D = BasicRoundCollider <true>;
		}
	}
}
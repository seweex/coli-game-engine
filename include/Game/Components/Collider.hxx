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
				public ComponentBase,
				public virtual Detail::PolymorphicCollider
			{
			public:
				void correct_on_start		  ()	  noexcept final {}
				void correct_on_late_update	  (float) noexcept final {}
				void correct_on_render_update ()	  noexcept final {}

				_NODISCARD static constexpr Detail::ComponentCategory get_category() noexcept {
					return Detail::ComponentCategory::collider;
				}
			};

			template <std::floating_point _FloatTy, bool _Use2D>
			class BasicBoxCollider final :
				public ColliderBase,
				public Geometry::BasicBoxCollider<_FloatTy, _Use2D>
			{
				using rotator_type = std::conditional_t<_Use2D, _FloatTy, glm::qua<_FloatTy>>;

			public:
				BasicBoxCollider(
					glm::vec <_Use2D ? 2 : 3, _FloatTy> const& size = glm::vec<_Use2D ? 2 : 3, _FloatTy>{ 1 },
					rotator_type const& rotator = {}
				) noexcept :
					Geometry::BasicBoxCollider<_FloatTy, _Use2D>(size, rotator)
				{}

				void on_update(float) final {
					if (!this->has_transform())
						take_dependencies();
				}

				void take_dependencies() final {
					this->bind_transform(get_owner().get_component<BasicTransform<_FloatTy, _Use2D>>());
				}

				_NODISCARD nlohmann::json serialize() const final {
					return static_cast<Geometry::BasicBoxCollider<_FloatTy, _Use2D> const&>(*this);
				}

				void deserialize(nlohmann::json const& obj) final {
					Geometry::BasicBoxCollider<_FloatTy, _Use2D>::operator=(obj);
				}
			};

			template <std::floating_point _FloatTy, bool _Use2D>
			class BasicRoundCollider final :
				public ColliderBase,
				public Geometry::BasicRoundCollider<_FloatTy, _Use2D>
			{
			public:
				BasicRoundCollider(_FloatTy radius = 1) noexcept :
					Geometry::BasicRoundCollider<_FloatTy, _Use2D>(radius)
				{}

				void on_update(float) final {
					if (!this->has_transform())
						take_dependencies();
				}

				void take_dependencies() final {
					this->bind_transform(get_owner().get_component<BasicTransform<_FloatTy, _Use2D>>());
				}

				_NODISCARD nlohmann::json serialize() const final {
					return static_cast<Geometry::BasicRoundCollider<_FloatTy, _Use2D> const&>(*this);
				}

				void deserialize(nlohmann::json const& obj) final {
					Geometry::BasicRoundCollider<_FloatTy, _Use2D>::operator=(obj);
				}
			};

			template <std::floating_point _FloatTy>
			using BoxCollider = BasicBoxCollider<_FloatTy, false>;

			template <std::floating_point _FloatTy>
			using BoxCollider2D = BasicBoxCollider<_FloatTy, true>;

			template <std::floating_point _FloatTy>
			using SphereCollider = BasicRoundCollider<_FloatTy, false>;

			template <std::floating_point _FloatTy>
			using CircleCollider2D = BasicRoundCollider<_FloatTy, true>;
		}
	}
}
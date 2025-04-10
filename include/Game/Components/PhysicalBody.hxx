#pragma once

#include "../../Common.hxx"
#include "../../Utility.hxx"

#include "../Component.hxx"
#include "Transform.hxx"

#include "../../Geometry/Collider.hxx"
#include "../../Geometry/PhysicalBody.hxx"

namespace Coli
{
	namespace Game
	{
		namespace Components
		{
			class PhysicalBodyBase :
				public ComponentBase,
				public virtual Detail::PolymorphicPhysicalBody
			{
			public:
				void correct_on_start		  ()	  noexcept final {}
				void correct_on_update		  (float) noexcept final {}
				void correct_on_render_update ()	  noexcept final {}

				_NODISCARD static constexpr Detail::ComponentCategory get_category() noexcept {
					return Detail::ComponentCategory::physical_body;
				}
			};

			template <std::floating_point _FloatTy, bool _Use2D>
			class BasicPhysicalBody final :
				public PhysicalBodyBase,
				public Geometry::BasicPhysicalBody<_FloatTy, _Use2D>
			{
			public:
				void on_update(float) final {
					if (!this->has_transform())
						take_dependencies();
				}

				void on_late_update (float time) noexcept final {
					this->apply_forces   (time);
					this->apply_velocity (time);
				}

				void take_dependencies() final {
					this->bind_transform(get_owner().get_component<BasicTransform<_FloatTy, _Use2D>>());
				}

				_NODISCARD nlohmann::json serialize() const final {
					return static_cast<Geometry::BasicPhysicalBody<_FloatTy, _Use2D> const&>(*this);
				}

				void deserialize(nlohmann::json const& obj) final {
					Geometry::BasicPhysicalBody<_FloatTy, _Use2D>::operator=(obj);
				}
			};

			template <std::floating_point _FloatTy>
			using PhysicalBody = BasicPhysicalBody<_FloatTy, false>;
			
			template <std::floating_point _FloatTy>
			using PhysicalBody2D = BasicPhysicalBody<_FloatTy, true>;
		}
	}
}
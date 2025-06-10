#pragma once

#include "../../Common.hxx"
#include "../../Utility.hxx"

#include "Transform.hxx"
#include "../Component.hxx"

#include "../../Geometry/Collider.hxx"
#include "../../Geometry/PhysicalBody.hxx"

namespace Coli
{
	namespace Game
	{
		namespace Components
		{
			class PhysicalBodyBase :
				public Detail::AssetBase,
				public ComponentBase
			{
			protected:
				PhysicalBodyBase() noexcept = default;

			public:
				void start()  noexcept final {}
				void render() noexcept final {}
				void on_update (float) noexcept final {}

				_NODISCARD static constexpr ComponentBase::Category get_category() noexcept {
					return ComponentBase::Category::physical_body;
				}
			};

			template <bool _Use2D>
			class BasicPhysicalBody final :
				public PhysicalBodyBase,
				public Geometry::BasicPhysicalBody <_Use2D>
			{
			public:
				void on_late_update (float time) noexcept final {
					this->apply_forces   (time);
					this->apply_velocity (time);
				}

				void on_restore(nlohmann::json const& obj) final
				{
					auto& base     = static_cast <Geometry::BasicPhysicalBody <_Use2D>&>(*this);
					auto  restored = static_cast <Geometry::BasicPhysicalBody <_Use2D>>(obj);

					base = restored;
				}

				_NODISCARD nlohmann::json on_save() const final
				{
					auto const& base = static_cast <Geometry::BasicPhysicalBody <_Use2D> const&>(*this);
					nlohmann::json object;

					object = base;
					return object;
				}
			};

			using PhysicalBody   = BasicPhysicalBody <false>;
			using PhysicalBody2D = BasicPhysicalBody <true>;
		}
	}
}
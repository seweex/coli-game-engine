#pragma once

#include "../../Common.hxx"
#include "../../Utility.hxx"

#include "../Component.hxx"
#include "../../Geometry/Transform.hxx"

namespace Coli
{
	namespace Game
	{
		namespace Components
		{
			template <bool _Use2D>
			class BasicTransform final :
				public Detail::AssetBase,
				public ComponentBase,
				public Geometry::BasicTransform <_Use2D>
			{
			public:
				void start ()  noexcept final {}
				void render () noexcept final {}
				void update (float) noexcept final {}

				void on_late_update(float) noexcept final {
					this->commit(*this);
				}
				
				void on_restore(nlohmann::json const& obj) final
				{
					auto& base     = static_cast <Geometry::BasicTransform <_Use2D>&>(*this);
					auto  restored = static_cast <Geometry::BasicTransform <_Use2D>>(obj);

					base = restored;
				}

				_NODISCARD nlohmann::json on_save() const final
				{
					auto const& base = static_cast <Geometry::BasicTransform <_Use2D> const&>(*this);
					nlohmann::json object;

					object = base;
					return object;
				}

				_NODISCARD static constexpr ComponentBase::Category get_category() noexcept {
					return ComponentBase::Category::transform;
				}
			};

			using Transform   = BasicTransform <false>;
			using Transform2D = BasicTransform <true>;
		}
	}
}
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
			template <std::floating_point _FloatTy, bool _Use2D>
			class BasicTransform final :
				public ComponentBase,
				public Geometry::BasicTransform <_FloatTy, _Use2D>
			{
			public:
				void correct_on_start		  ()	  noexcept final {}
				void correct_on_update		  (float) noexcept final {}
				void correct_on_render_update ()	  noexcept final {}

				void on_late_update(float) noexcept final {
					this->commit_change();
				}

				_NODISCARD static constexpr Detail::ComponentCategory get_category() noexcept {
					return Detail::ComponentCategory::transform;
				}

				_NODISCARD nlohmann::json serialize() const final {
					return static_cast<Geometry::BasicTransform<_FloatTy, _Use2D> const&>(*this);
				}

				void deserialize(nlohmann::json const& obj) final {
					Geometry::BasicTransform<_FloatTy, _Use2D>::operator=(obj);
				}
			};

			template <std::floating_point _FloatTy>
			using Transform = BasicTransform<_FloatTy, false>;

			template <std::floating_point _FloatTy>
			using Transform2D = BasicTransform<_FloatTy, true>;
		}
	}
}
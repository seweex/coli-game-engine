#pragma once

#include "../../Common.hxx"
#include "../../Utility.hxx"

#include "Transform.hxx"
#include "../../Graphics/Drawable.hxx"
#include "../../Graphics/Renderer.hxx"

namespace Coli
{
	namespace Game
	{
		namespace Components
		{
			template <std::floating_point _FloatTy, bool _Use2D>
			class BasicDrawable :
				public  Game::ComponentBase,
				private Graphics::Drawable<Geometry::BasicVertex<_FloatTy, _Use2D>>
			{
				using _drawable_base = Graphics::Drawable<Geometry::BasicVertex<_FloatTy, _Use2D>>;

			public:
				using _drawable_base::set_material;

				BasicDrawable(Geometry::Mesh<Geometry::BasicVertex<_FloatTy, _Use2D>> const& mesh) :
					_drawable_base (mesh)
				{}

				void correct_on_start		()	    noexcept final {}
				void correct_on_late_update (float) noexcept final {}

				void on_update(float) final {
					if (auto transform = myTransform.lock())
						this->update(*transform);
					else
						take_dependencies();
				}

				void on_render_update() final {
					myRenderer->draw(*this);
				}

				void take_dependencies() final {
					myTransform = get_owner().get_component<BasicTransform<_FloatTy, _Use2D>>();
				}

				_NODISCARD static constexpr Detail::ComponentCategory get_category() noexcept {
					return Detail::ComponentCategory::drawable;
				}

				_NODISCARD nlohmann::json serialize() const noexcept final { return {}; }
				void deserialize(nlohmann::json const&)     noexcept final {}

			private:
				using _drawable_base::myRenderer;

				std::weak_ptr<Geometry::BasicTransform<_FloatTy, _Use2D> const> myTransform;
			};

			template <std::floating_point _FloatTy>
			using Drawable = BasicDrawable<_FloatTy, false>;
			
			template <std::floating_point _FloatTy>
			using Drawable2D = BasicDrawable<_FloatTy, true>;
		}
	}
}
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
			template <bool _Use2D>
			class BasicDrawable :
				public  Game::ComponentBase,
				private Graphics::Drawable <Geometry::BasicVertex<_Use2D>>
			{
				using base = Graphics::Drawable<Geometry::BasicVertex<_Use2D>>;

			public:
				using base::set_material;

				BasicDrawable(Geometry::Mesh <Geometry::BasicVertex <_Use2D>> const& mesh) :
					base (mesh)
				{}

				void start		 ()	     noexcept final {}
				void late_update (float) noexcept final {}

				void on_update (float) final 
				{
					if (auto transform = myTransform.lock())
						base::update(*transform);

					else {
						auto& owner = get_owner();
						myTransform = owner.get_component <Game::Components::BasicTransform <_Use2D>>();
					}						
				}

				void on_render() final {
					if (auto renderer = this->get_renderer())
						renderer->draw(*this);
				}

				_NODISCARD static constexpr ComponentBase::Category get_category() noexcept {
					return ComponentBase::Category::drawable;
				}

			private:
				std::weak_ptr <Geometry::BasicTransform <_Use2D> const> myTransform;
			};

			using Drawable   = BasicDrawable <false>;
			using Drawable2D = BasicDrawable <true>;
		}
	}
}
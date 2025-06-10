#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Context.hxx"
#include "Drawable.hxx"

#include "../Visual/Camera.hxx"

namespace Coli
{
	namespace Detail
	{
		inline namespace OpenGL
		{
			struct CameraUniformBlock
			{
				static constexpr unsigned binding_index = 0;

				glm::mat4 view = { 1.f };
				glm::mat4 proj = { 1.f };
			};

			class CameraContext
			{
			public:
				CameraContext() :
					myBuffer (default_value)
				{}

				CameraContext(CameraContext&&)		= delete;
				CameraContext(CameraContext const&) = delete;

				CameraContext& operator=(CameraContext&&)	   = delete;
				CameraContext& operator=(CameraContext const&) = delete;

				void update (CameraBase const& camera) noexcept 
				{
					auto const projChanged = camera.has_proj_changed();
					auto const viewChanged = camera.has_view_changed();

					if (projChanged && viewChanged)
						myBuffer.write(CameraUniformBlock{ camera.get_view_matrix(),
														   camera.get_projection_matrix() }, 0);

					else if (projChanged)
						myBuffer.write (camera.get_projection_matrix(), 
									    offsetof(CameraUniformBlock, proj));

					else if (viewChanged)
						myBuffer.write (camera.get_view_matrix(), 
									    offsetof(CameraUniformBlock, view));
				}

				void bind() {
					myBuffer.bind(CameraUniformBlock::binding_index);
				}

				static void unbind() noexcept {
					Graphics::UniformBuffer::unbind(CameraUniformBlock::binding_index);
				}

			private:
				static constexpr CameraUniformBlock default_value = {};

				Graphics::UniformBuffer myBuffer;
			};
		}
	}

	namespace Graphics
	{
		class Renderer final :
			public Detail::CameraContext
		{
		public:	
			Renderer() = default;

			Renderer(Renderer&&)	  = delete;
			Renderer(Renderer const&) = delete;

			Renderer& operator=(Renderer&&)		 = delete;
			Renderer& operator=(Renderer const&) = delete;

			template <class _VertexTy>
			void draw (Drawable<_VertexTy>& drawable)
			{
				Detail::CameraContext::bind();
				drawable.bind();

				glDrawElements (GL_TRIANGLES, drawable.get_vertices_count(), GL_UNSIGNED_INT, 0);

				drawable.unbind();
				Detail::CameraContext::unbind();
			}
		};
	}
}
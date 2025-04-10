#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Texture.hxx"
#include "Program.hxx"

namespace Coli
{
	namespace Detail
	{
		class MaterialBase
		{
		protected:
			MaterialBase(
				std::string_view vertexCode,
				std::string_view fragmentCode,
				std::optional<std::string_view> geometryCode = std::nullopt
			) {
				Graphics::VertexShader   vertex   { vertexCode };
				Graphics::FragmentShader fragment { fragmentCode };

				if (geometryCode) {
					Graphics::GeometryShader geometry { fragmentCode };
					myProgram = std::make_shared<Graphics::Program>(vertex, fragment, geometry);
				}
				else
					myProgram = std::make_shared<Graphics::Program>(vertex, fragment);				
			}

		public:
			virtual void   bind_textures() = 0;
			virtual void unbind_textures() noexcept = 0;

			void bind() {
				myProgram->bind();
				bind_textures();
			}

			void unbind() noexcept {
				unbind_textures();
				Graphics::Program::unbind();
			}

		private:
			std::shared_ptr<Graphics::Program> myProgram;
		};
	}

	namespace Graphics
	{
		class DefaultMaterial final :
			public Detail::MaterialBase
		{
		public:
			DefaultMaterial(Visual::Texture const& texture) :
				Detail::MaterialBase(
					Detail::default_vertex_shader,
					Detail::default_pixel_shader
				),
				myTexture (std::make_shared<Texture>(texture))
			{}

			void bind_textures() final {
				myTexture->bind(texture_binding);
			}

			void unbind_textures() noexcept final {
				Texture::unbind(texture_binding);
			}

		private:
			static constexpr unsigned texture_binding = 0;

			std::shared_ptr<Texture> myTexture;
		};
	}
}
#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "../Visual/Texture.hxx"

namespace Coli
{
	namespace Graphics
	{
		class Texture final :
			public Detail::ContextDependBase
		{
			static void x_failed_create() {
				throw std::runtime_error("Failed to create a texture");
			}

			static void x_invalid_index() {
				throw std::invalid_argument("Invalid index");
			}

		public:
			Texture (Visual::Texture const& texture)
			{
				glCreateTextures(GL_TEXTURE_2D, 1, &myHandle);

				if (myHandle == 0)
					x_failed_create();

				auto [width, height] = texture.size();

				glBindTexture(GL_TEXTURE_2D, myHandle);

				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

				glTexStorage2D  (GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
				glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, texture.data());

				glGenerateMipmap (GL_TEXTURE_2D);
				glBindTexture    (GL_TEXTURE_2D, 0);
			}

			~Texture() noexcept {
				glDeleteTextures(1, &myHandle);
			}

			Texture(Texture&&)	    = delete;
			Texture(Texture const&) = delete;

			Texture& operator=(Texture&&)	   = delete;
			Texture& operator=(Texture const&) = delete;

			void bind(unsigned index)
			{
				if (index < indexed_bindings.size()) {
					if (indexed_bindings [index] != myHandle) {
						glActiveTexture (GL_TEXTURE0 + index);
						glBindTexture   (GL_TEXTURE_2D, indexed_bindings[index] = myHandle);
					}
				}
				else
					x_invalid_index();
			}

			static void unbind(unsigned index) noexcept
			{
				if (index < indexed_bindings.size()) {
					glActiveTexture (GL_TEXTURE0 + index);
					glBindTexture   (GL_TEXTURE_2D, indexed_bindings[index] = 0);
				}
			}

		private:
			static inline std::array<GLuint, 5> indexed_bindings = { 0 };
			static inline GLuint				current_binding  = 0;

			GLuint myHandle = 0;
		};
	}
}
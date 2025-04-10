#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "../Visual/Texture.hxx"

namespace Coli
{
	namespace Graphics
	{
		class Texture final
		{
		public:
			Texture(Visual::Texture const& texture)
			{
				if (!Context::is_ready())
					throw std::runtime_error("no ready context");

				glCreateTextures(GL_TEXTURE_2D, 1, &myHandle);

				if (myHandle == 0)
					throw std::runtime_error("failed to create a texture");

				auto [width, height] = texture.get_sizes();

				glBindTexture(GL_TEXTURE_2D, myHandle);

				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

				glTexStorage2D  (GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
				glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, texture.get_data());

				glGenerateMipmap (GL_TEXTURE_2D);
				glBindTexture (GL_TEXTURE_2D, 0);
			}

			~Texture() noexcept {
				glDeleteTextures(1, &myHandle);
			}
			
			void bind(unsigned index)
			{
				if (index < indexed_bindings.size()) _LIKELY {
					if (indexed_bindings[index] != myHandle) _LIKELY
					{
						glActiveTexture (GL_TEXTURE0 + index);
						glBindTexture   (GL_TEXTURE_2D, indexed_bindings[index] = myHandle);
					}
				}
				else
					throw std::invalid_argument("invalid index");
			}

			static void unbind(unsigned index) noexcept
			{
				if (index < indexed_bindings.size()) _LIKELY {
					glActiveTexture (GL_TEXTURE0 + index);
					glBindTexture   (GL_TEXTURE_2D, indexed_bindings[index] = 0);
				}
			}

		private:
			static inline std::array<GLuint, 5> indexed_bindings = { 0 };
			static inline GLuint				current_binding = 0;

			GLuint myHandle = 0;
		};
	}
}
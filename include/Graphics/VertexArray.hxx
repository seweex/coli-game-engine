#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Buffer.hxx"
#include "../Geometry/Vertex.hxx"

namespace Coli
{
	namespace Graphics 
	{
		template <Detail::Vertex _VertexTy>
		class VertexArray final
		{
		public:
			VertexArray(Detail::VertexBufferBase& vertices) 
			{
				if (!Context::is_ready())
					throw std::runtime_error("no ready context");

				glGenVertexArrays(1, &myHandle);

				if (myHandle == 0)
					throw std::runtime_error("failed to create a vertex array");

				bind();
				vertices.bind();

				using traits_type = Detail::VertexTraits<_VertexTy>;

				glVertexAttribPointer(
					position_index,
					traits_type::position_length(),
					traits_type::float_type_enum(),
					false,
					traits_type::vertex_size(),
					reinterpret_cast<void const*>(traits_type::position_offset())
				);
				glVertexAttribPointer(
					texcoord_index,
					traits_type::texcoord_length(),
					traits_type::float_type_enum(),
					false,
					traits_type::vertex_size(),
					reinterpret_cast<void const*>(traits_type::texcoord_offset())
				);

				glEnableVertexAttribArray(position_index);
				glEnableVertexAttribArray(texcoord_index);

				vertices.unbind();
				unbind();
			}

			~VertexArray() noexcept {
				glDeleteVertexArrays(1, &myHandle);
			}

			void bind() noexcept {
				if (myHandle != current_binding) _LIKELY
					glBindVertexArray(current_binding = myHandle);
			}

			static void unbind() noexcept {
				glBindVertexArray(current_binding = 0);
			}

		private:
			static constexpr unsigned position_index = 0;
			static constexpr unsigned texcoord_index = 1;

			static inline GLuint current_binding = 0;

			GLuint myHandle;
		};
	}
}
#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Shader.hxx"

namespace Coli
{
	namespace Graphics
	{
		class Program final
		{
		public:
			Program(
				VertexShader&   vertex,
				FragmentShader& fragment
			) {
				if (!Context::is_ready())
					throw std::runtime_error("no ready context");

				if ((myHandle = glCreateProgram()) == 0)
					throw std::runtime_error("failed to create a program");

				glAttachShader (myHandle, vertex.myHandle);
				glAttachShader (myHandle, fragment.myHandle);

				glLinkProgram (myHandle);
				
				glDetachShader (myHandle, fragment.myHandle);
				glDetachShader (myHandle, vertex.myHandle);

				GLint flag;
				glGetProgramiv(myHandle, GL_LINK_STATUS, &flag);

				if (flag == GL_FALSE)
					throw std::runtime_error("failed to link the program");
			}

			Program(
				VertexShader& vertex,
				FragmentShader& fragment,
				GeometryShader& geometry
			) {
				if (!Context::is_ready())
					throw std::runtime_error("no ready context");

				if ((myHandle = glCreateProgram()) == 0)
					throw std::runtime_error("failed to create a program");

				glAttachShader(myHandle, vertex.myHandle);
				glAttachShader(myHandle, fragment.myHandle);
				glAttachShader(myHandle, geometry.myHandle);

				glLinkProgram(myHandle);

				glDetachShader(myHandle, geometry.myHandle);
				glDetachShader(myHandle, fragment.myHandle);
				glDetachShader(myHandle, vertex.myHandle);

				GLint flag;
				glGetProgramiv(myHandle, GL_LINK_STATUS, &flag);

				if (flag == GL_FALSE)
					throw std::runtime_error("failed to link the program");
			}

			~Program() noexcept {
				glDeleteProgram(myHandle);
			}

			void bind() noexcept {
				if (myHandle != current_binding)
					glUseProgram(current_binding = myHandle);
			}

			static void unbind() noexcept {
				glUseProgram(current_binding = 0);
			}

		private:
			static inline GLuint current_binding = 0;

			GLuint myHandle = 0;
		};
	}
}
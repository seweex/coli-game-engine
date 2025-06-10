#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Shader.hxx"

namespace Coli
{
	namespace Graphics
	{
		class Program final :
			public Detail::ContextDependBase
		{
			static void x_failed_create() {
				throw std::runtime_error("Failed to create a program");
			}

			static void x_failed_link() {
				throw std::runtime_error("Failed to link the program");
			}

		public:
			Program (
				VertexShader&   vertex,
				FragmentShader& fragment
			) {
				if ((myHandle = glCreateProgram()) == 0)
					x_failed_create();

				glAttachShader (myHandle, vertex.myHandle);
				glAttachShader (myHandle, fragment.myHandle);

				glLinkProgram (myHandle);
				
				glDetachShader (myHandle, fragment.myHandle);
				glDetachShader (myHandle, vertex.myHandle);

				GLint flag;
				glGetProgramiv(myHandle, GL_LINK_STATUS, &flag);

				if (flag == GL_FALSE)
					x_failed_link();
			}

			Program (
				VertexShader&   vertex,
				FragmentShader& fragment,
				GeometryShader& geometry
			) {
				if ((myHandle = glCreateProgram()) == 0)
					x_failed_create();

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
					x_failed_link();
			}

			~Program() noexcept {
				glDeleteProgram(myHandle);
			}

			Program(Program&&)      = delete;
			Program(Program const&) = delete;

			Program& operator=(Program&&)	   = delete;
			Program& operator=(Program const&) = delete;

			void bind() noexcept {
				if (myHandle != current_binding)
					glUseProgram (current_binding = myHandle);
			}

			static void unbind() noexcept {
				glUseProgram (current_binding = 0);
			}

		private:
			static inline GLuint current_binding = 0;

			GLuint myHandle = 0;
		};
	}
}
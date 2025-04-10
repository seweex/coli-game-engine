#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

namespace Coli
{
	namespace Graphics
	{
		inline namespace OpenGL
		{
			class Context final
			{
				static void load_opengl() {
					if (!opengl_loaded) 
					{
						if (!gladLoadGL())
							throw std::runtime_error("failed to load OpenGL");
						
						opengl_loaded = true;
					}
				}

			public:
				Context() {
					if (!context_exists) 
					{
						if (!glfwInit())
							throw std::runtime_error("failed to init context");

						glfwWindowHint(GLFW_VISIBLE, false);
						glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
						glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
						glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

						context_exists = true;
					}
					else
						throw std::runtime_error("another context already exist");
				}

				~Context() noexcept {
					glfwTerminate();
				}

				Context(Context const&)			   = delete;
				Context& operator=(Context const&) = delete;

				_NODISCARD static bool exists() noexcept {
					return context_exists;
				}

				_NODISCARD static bool is_ready() noexcept {
					return opengl_loaded && context_exists;
				}

				friend class Window;

			private:
				static inline bool context_exists = false;
				static inline bool opengl_loaded  = false;
			};
		}
	}
}
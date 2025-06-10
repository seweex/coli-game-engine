#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

namespace Coli
{
	namespace Detail 
	{
		inline namespace OpenGL 
		{
			class ResourceBase;

			class GLADContext 
			{
				static void x_failed_load() {
					throw std::runtime_error("Failed to load GLAD");
				}

			protected:
				GLADContext() noexcept = default;

				static void load() {
					if (!loaded) {
						if (!gladLoadGL())
							x_failed_load();

						loaded = true;
					}
				}

			public:
				GLADContext(GLADContext&&)		= delete;
				GLADContext(GLADContext const&) = delete;

				GLADContext& operator=(GLADContext&&)	   = delete;
				GLADContext& operator=(GLADContext const&) = delete;

				_NODISCARD static bool is_loaded() noexcept {
					return loaded;
				}

			protected:
				static inline bool loaded = false;
			};

			class GLFWContext
			{
				static void x_failed_init() {
					throw std::runtime_error("Failed to initialize GLFW");
				}

			protected:
				GLFWContext() noexcept = default;

				static void initialize() {
					if (!initialized) {
						if (!glfwInit())
							x_failed_init();

						initialized = true;
					}
				}

			public:
				~GLFWContext() noexcept {
					if (initialized) {
						glfwTerminate();
						initialized = false;
					}
				}

				GLFWContext(GLFWContext&&)	    = delete;
				GLFWContext(GLFWContext const&) = delete;

				GLFWContext& operator=(GLFWContext&&)	   = delete;
				GLFWContext& operator=(GLFWContext const&) = delete;

				_NODISCARD static bool is_initialized() noexcept {
					return initialized;
				}
				
			private:
				static inline bool initialized = false;
			};
		}
	}

	namespace Graphics
	{
		inline namespace OpenGL
		{
			class Context final :
				public  Detail::SingletonBase <Context>,
				private Detail::OpenGL::GLFWContext,
				private Detail::OpenGL::GLADContext
			{
			public:
				Context() {
					GLFWContext::initialize();
				}

				Context(Context&&)		= delete;
				Context(Context const&) = delete;

				Context& operator=(Context&&)	   = delete;
				Context& operator=(Context const&) = delete;

				_NODISCARD static bool is_ready() noexcept {
					return GLADContext::is_loaded() && GLFWContext::is_initialized();
				}
			
			private:
				static void load() {
					GLADContext::load();
				}

				friend class Window;
			};
		}
	}

	namespace Detail
	{
		inline namespace OpenGL
		{
			class ContextDependBase
			{
			public:
				ContextDependBase() {
					if (!Graphics::Context::is_ready())
						throw std::runtime_error("No context ready");
				}
			};
		}
	}
}
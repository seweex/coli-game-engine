#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Context.hxx"

#include "../Input/Map.hxx"

namespace Coli
{
	namespace Graphics
	{
		inline namespace OpenGL 
		{
			class Window;
		}
	}

	namespace Detail
	{
		inline namespace OpenGL
		{
			class WindowHandle
			{
				static void x_no_context() {
					throw std::runtime_error("Context doesn't exists");
				}

				static void x_failed_create_window() {
					throw std::runtime_error("Failed to create a window");
				}

			protected:
				using user_type = Graphics::OpenGL::Window;

				WindowHandle() noexcept = default;

				WindowHandle (std::string_view title, int width, int height)
				{
					if (Graphics::OpenGL::Context::exist())
					{
						myHandle = glfwCreateWindow (width, 
							height, title.data(), nullptr, nullptr);

						if (myHandle) {
							glfwMakeContextCurrent (myHandle);
							glfwSetWindowAttrib	   (myHandle, GLFW_RESIZABLE, false);
							glfwSetInputMode	   (myHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
						}
						else
							x_failed_create_window();
					}
					else
						x_no_context();
				}
			
			public:
				~WindowHandle() noexcept {
					if (myHandle)
						glfwDestroyWindow(myHandle);
				}

				WindowHandle(WindowHandle&&)	  = delete;
				WindowHandle(WindowHandle const&) = delete;

				WindowHandle& operator=(WindowHandle&&)		 = delete;
				WindowHandle& operator=(WindowHandle const&) = delete;

			protected:
				void set_user_pointer (user_type* ptr) noexcept {
					if (myHandle)
						glfwSetWindowUserPointer(myHandle, ptr);						
				}

				void bind_input_handlers (
					GLFWkeyfun		   key,
					GLFWmousebuttonfun mouse,
					GLFWcursorposfun   cursor,
					GLFWscrollfun	   scroll
				) noexcept
				{
					glfwSetKeyCallback		   (myHandle, key);
					glfwSetMouseButtonCallback (myHandle, mouse);
					glfwSetCursorPosCallback   (myHandle, cursor);
					glfwSetScrollCallback	   (myHandle, scroll);
				}

				_NODISCARD GLFWwindow* get_handle() const noexcept {
					return myHandle;
				}

			private:
				GLFWwindow* myHandle = nullptr;
			};

			class WindowInputBase :
				public virtual WindowHandle
			{
				using typename WindowHandle::user_type;

				static void key_callback (GLFWwindow* handle, 
					int key, int scancode, int act, int mods
				) noexcept;

				static void mouse_callback (GLFWwindow* handle,
					int btn, int act, int mode
				) noexcept;

				static void cursor_callback (GLFWwindow* handle, 
					double xPos, double yPos
				) noexcept;

				static void wheel_callback (GLFWwindow* handle,
					double xOffset, double yOffset
				) noexcept;

			protected:
				WindowInputBase() noexcept {
					this->bind_input_handlers (& key_callback,
						& mouse_callback, & cursor_callback, & wheel_callback);
				}

			public:
				WindowInputBase(WindowInputBase&&)		= delete;
				WindowInputBase(WindowInputBase const&) = delete;

				WindowInputBase& operator=(WindowInputBase&&)	   = delete;
				WindowInputBase& operator=(WindowInputBase const&) = delete;

				_NODISCARD void bind_input_map (std::weak_ptr <Input::Map> inputMap) noexcept {
					myInputMap.swap(inputMap);
				}

			protected:
				std::weak_ptr <Input::Map> myInputMap;
			};
		}
	}

	namespace Graphics
	{
		inline namespace OpenGL
		{
			class Window final :
				public virtual Detail::OpenGL::WindowHandle,
				public		   Detail::OpenGL::WindowInputBase
			{
				void configure() noexcept 
				{
					glClearColor (0, 0, 0, 1);

					auto [width, height] = size();

					glScissor  (0, 0, width, height);
					glViewport (0, 0, width, height);

					glEnable (GL_DEPTH_TEST);
					glEnable (GL_SCISSOR_TEST);
					//glEnable (GL_CULL_FACE);
					glDisable (GL_CULL_FACE);

					glCullFace  (GL_BACK);
					glFrontFace (GL_CW);
				}

			public:
				struct Configuration {
					std::string title = "Untitled";
					int			width  = 640;
					int			height = 480;
				};

				Window (Configuration const& config) :
					WindowHandle (config.title, config.width, config.height)
				{
					Context::load();
					configure();

					set_user_pointer(this);
				}
				
				Window(Window&&)	  = delete;
				Window(Window const&) = delete;

				Window& operator=(Window&&)		 = delete;
				Window& operator=(Window const&) = delete;

				_NODISCARD std::pair<int, int> size() const noexcept
				{
					auto handle = get_handle();
					std::pair <int, int> sizes;

					glfwGetWindowSize(handle, std::addressof(sizes.first),
											  std::addressof(sizes.second));
					return sizes;
				}

				_NODISCARD bool should_close() const noexcept 
				{
					auto handle = get_handle();

					if (handle)
						return glfwWindowShouldClose(handle);
					else
						return true;
				}

				void update() noexcept 
				{
					constexpr auto clear_mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
					auto handle = get_handle();

					glfwPollEvents  ();
					glfwSwapBuffers (handle);
					glClear			(clear_mask);
									
				}

				void show() noexcept {
					auto handle = get_handle();
					glfwShowWindow (handle);
				}

				void hide() noexcept {
					auto handle = get_handle();
					glfwHideWindow (handle);
				} 
			};
		}
	}

	namespace Detail
	{
		inline namespace OpenGL
		{
			void WindowInputBase::key_callback (GLFWwindow* handle,
				int key, int scancode, int act, int mods
			) noexcept
			{
				auto userPtr	  = static_cast<user_type*>(glfwGetWindowUserPointer(handle));
				auto inputHandler = userPtr->myInputMap.lock();

				if (inputHandler)
				{
					Input::Button button = static_cast<Input::KeyboardButton>(key);
					Input::Action action;

					switch (act)
					{
					case GLFW_PRESS: 
						action = Input::Action::pressed;
						break;

					case GLFW_REPEAT:
						action = Input::Action::hovered;
						break;

					case GLFW_RELEASE:
						action = Input::Action::released;
						break;

					default:
						return;
					}

					Input::ButtonEvent event { button, action };
					inputHandler->handle(event);
				}			
			}

			void WindowInputBase::mouse_callback (GLFWwindow* handle,
				int btn, int act, int mode
			) noexcept
			{
				auto userPtr	  = static_cast<user_type*>(glfwGetWindowUserPointer(handle));
				auto inputHandler = userPtr->myInputMap.lock();

				if (inputHandler)
				{
					Input::MouseButton button;
					Input::Action	   action;

					switch (act)
					{
					case GLFW_PRESS:
						action = Input::Action::pressed;
						break;

					case GLFW_REPEAT:
						action = Input::Action::hovered;
						break;

					case GLFW_RELEASE:
						action = Input::Action::released;
						break;

					default:
						return;
					}

					switch (btn)
					{
					case GLFW_MOUSE_BUTTON_LEFT: 
						button = Input::MouseButton::left;
						break;

					case GLFW_MOUSE_BUTTON_MIDDLE:
						button = Input::MouseButton::middle;
						break;

					case GLFW_MOUSE_BUTTON_RIGHT:
						button = Input::MouseButton::right;
						break;

					case GLFW_MOUSE_BUTTON_4:
						button = Input::MouseButton::x1;
						break;

					case GLFW_MOUSE_BUTTON_5:
						button = Input::MouseButton::x2;
						break;

					default: 
						return;
					}

					Input::ButtonEvent event { button, action };
					inputHandler->handle(event);
				}
			}

			void WindowInputBase::cursor_callback (GLFWwindow* handle,
				double xPos, double yPos
			) noexcept 
			{
				auto userPtr      = static_cast<user_type*>(glfwGetWindowUserPointer(handle));
				auto inputHandler = userPtr->myInputMap.lock();

				if (inputHandler) 
				{
					Input::MouseEvent event{ xPos, yPos };
					inputHandler->handle(event);
				}
			}

			void WindowInputBase::wheel_callback (GLFWwindow* handle,
				double xOffset, double yOffset
			) noexcept
			{
				auto userPtr      = static_cast<user_type*>(glfwGetWindowUserPointer(handle));
				auto inputHandler = userPtr->myInputMap.lock();

				if (inputHandler)
				{
					Input::MouseEvent event { yOffset };
					inputHandler->handle(event);
				}
			}
		}
	}
}

namespace nlohmann
{
	template <>
	struct adl_serializer <Coli::Graphics::Window::Configuration>
	{
	private:
		struct Keys {
			static constexpr std::string_view title  = "title";
			static constexpr std::string_view width  = "width";
			static constexpr std::string_view height = "height";
		};

	public:
		static void to_json (json& j, Coli::Graphics::Window::Configuration const& val)
		{
			j[Keys::title]  = val.title;
			j[Keys::width]  = val.width;
			j[Keys::height] = val.height;
		}

		static void from_json (const json& j, Coli::Graphics::Window::Configuration& val)
		{
			using Coli::Detail::Json::try_fill;

			decltype (val.title) tempTitle;
			try_fill (j, tempTitle, Keys::title);

			decltype (val.width) tempWidth;
			try_fill (j, tempWidth, Keys::width);

			decltype (val.height) tempHeight;
			try_fill (j, tempHeight, Keys::height);

			val.title  = std::move(tempTitle);
			val.width  = tempWidth;
			val.height = tempHeight;
		}
	};
}
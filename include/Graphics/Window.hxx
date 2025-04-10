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
			class Window final
			{
				static void key_callback(GLFWwindow* handle, 
					int key, int scancode, int action, int mods
				) noexcept
				{
					auto& inputHandler =  static_cast<Window*>(glfwGetWindowUserPointer(handle))
									   -> myInputHandler;
					
					switch (action)
					{
					case GLFW_PRESS: _FALLTHROUGH;
					case GLFW_REPEAT:
						inputHandler.handle_input(key, Detail::Action::pressed);
						break;

					case GLFW_RELEASE:
						inputHandler.handle_input(key, Detail::Action::released);
						break;
					}
				}

				static void mouse_callback(GLFWwindow* handle,
					int inButton, int inAction, int mode
				) noexcept
				{
					auto& inputHandler =  static_cast<Window*>(glfwGetWindowUserPointer(handle))
									   -> myInputHandler;

					Detail::MouseButton button;
					Detail::Action	    action;

					switch (inButton)
					{
					case GLFW_MOUSE_BUTTON_LEFT: 
						button = Detail::MouseButton::left;
						break;

					case GLFW_MOUSE_BUTTON_MIDDLE:
						button = Detail::MouseButton::middle;
						break;

					case GLFW_MOUSE_BUTTON_RIGHT:
						button = Detail::MouseButton::right;
						break;

					case GLFW_MOUSE_BUTTON_4:
						button = Detail::MouseButton::x1;
						break;

					case GLFW_MOUSE_BUTTON_5:
						button = Detail::MouseButton::x2;
						break;

					default: return;
					}

					switch (inAction)
					{
					case GLFW_PRESS:
						action = Detail::Action::pressed;
						break;

					case GLFW_RELEASE:
						action = Detail::Action::released;
						break;

					default: return;
					}

					inputHandler.handle_input(button, action);
				}

				static void cursor_callback(GLFWwindow* handle, 
					double xPos, double yPos
				) noexcept 
				{
					auto& inputHandler =  static_cast<Window*>(glfwGetWindowUserPointer(handle))
									   -> myInputHandler;

					inputHandler.handle_input(xPos, yPos);
				}

				static void wheel_callback(GLFWwindow* handle,
					double xOffset, double yOffset
				) noexcept
				{
					auto& inputHandler =  static_cast<Window*>(glfwGetWindowUserPointer(handle))
									   -> myInputHandler;

					inputHandler.handle_input(yOffset);
				}

				void configure() noexcept 
				{
					glClearColor(0, 0, 0, 1);

					auto const [width, height] = get_sizes();

					glScissor  (0, 0, width, height);
					glViewport (0, 0, width, height);

					glEnable (GL_DEPTH_TEST);
					glEnable (GL_SCISSOR_TEST);
					glEnable (GL_CULL_FACE);

					glCullFace  (GL_BACK);
					glFrontFace (GL_CW);
				}

			public:
				Window(Input::Map& inputMap, std::string_view title, int width, int height) :
					myInputHandler (inputMap)
				{
					if (!Context::exists())
						throw std::runtime_error("context doesn't exists");

					if (myHandle = glfwCreateWindow(width, height, title.data(), nullptr, nullptr))
					{
						glfwMakeContextCurrent   (myHandle);
						glfwSetWindowUserPointer (myHandle, this);

						glfwSetWindowAttrib		   (myHandle, GLFW_RESIZABLE, false);
						glfwSetKeyCallback		   (myHandle, key_callback);
						glfwSetMouseButtonCallback (myHandle, mouse_callback);
						glfwSetCursorPosCallback   (myHandle, cursor_callback);
						glfwSetScrollCallback	   (myHandle, wheel_callback);
						glfwSetInputMode		   (myHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

						glfwSwapInterval (0);

						Context::load_opengl();
						configure();
					}
					else
						throw std::runtime_error("failed to create a window");
				}

				~Window() noexcept {
					if (myHandle)
						glfwDestroyWindow(myHandle);
				}
				
				Window(Window const&)		     = delete;
				Window& operator=(Window const&) = delete;

				_NODISCARD std::pair<int, int> get_sizes() const noexcept
				{
					std::pair<int, int> sizes;
					glfwGetWindowSize(myHandle, std::addressof(sizes.first), 
												std::addressof(sizes.second));
					return sizes;
				}

				_NODISCARD bool should_close() const noexcept {
					return glfwWindowShouldClose(myHandle);
				}

				void update() noexcept {
					glfwPollEvents();
					glfwSwapBuffers(myHandle);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				}

				void show() noexcept {
					glfwShowWindow(myHandle);
				}

			private:
				Input::Map& myInputHandler;
				GLFWwindow* myHandle;
			};
		}
	}
}
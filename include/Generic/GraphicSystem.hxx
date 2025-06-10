#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "../Graphics/Context.hxx"
#include "../Graphics/Window.hxx"
#include "../Graphics/Renderer.hxx"

namespace Coli
{
	namespace Generic
	{
		class GraphicSystem final
		{
		public:
			GraphicSystem (Graphics::Window::Configuration const& config) :
				myWindow   (config),
				myRenderer (std::make_shared <Graphics::Renderer>())
			{
				Detail::DrawableBase::set_renderer(myRenderer);
			}

			GraphicSystem(GraphicSystem&&)	    = delete;
			GraphicSystem(GraphicSystem const&) = delete;

			GraphicSystem& operator=(GraphicSystem&&)	   = delete;
			GraphicSystem& operator=(GraphicSystem const&) = delete;
			
			_NODISCARD Graphics::Window const& get_window() const noexcept {
				return myWindow;
			}

			_NODISCARD Graphics::Window& get_window() noexcept {
				return myWindow;
			}

		private:
			Graphics::Context  myContext;
			Graphics::Window   myWindow;

			std::shared_ptr <Graphics::Renderer> myRenderer;
		};
	}
}
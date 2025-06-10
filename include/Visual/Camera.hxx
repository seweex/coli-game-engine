#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

namespace Coli
{
	namespace Detail
	{
		class CameraBase
		{
		protected:
			CameraBase() noexcept = default;

		public:
			_NODISCARD virtual bool has_view_changed() const noexcept = 0;

			_NODISCARD virtual glm::mat4       get_view_matrix() const noexcept = 0;
			_NODISCARD virtual glm::mat4 get_projection_matrix() const noexcept = 0;

			void set_fov(float fov) noexcept {
				myFOV		   = fov;
				hasProjChanged = true;
			}

			void set_aspect(float aspect) noexcept {
				myAspect       = aspect;
				hasProjChanged = true;
			}

			_NODISCARD float get_fov() const noexcept {
				return myFOV;
			}

			_NODISCARD bool has_proj_changed() const noexcept {
				return hasProjChanged;
			}

		protected:
			float myAspect;
			float myFOV;

			bool hasProjChanged;
		};
	}
}
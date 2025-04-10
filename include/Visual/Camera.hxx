#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

namespace Coli
{
	namespace Detail
	{
		class CameraBase
		{
		public:
			virtual void update_aspect(float aspect) noexcept = 0;

			_NODISCARD virtual bool has_view_changed() const noexcept = 0;
			_NODISCARD virtual bool has_proj_changed() const noexcept = 0;

			_NODISCARD virtual glm::mat4       get_view_matrix() const noexcept = 0;
			_NODISCARD virtual glm::mat4 get_projection_matrix() const noexcept = 0;
		};
	}
}
#pragma once

#include "../../Common.hxx"
#include "../../Utility.hxx"

#include "../Component.hxx"
#include "../Components/Transform.hxx"

#include "../../Visual/Camera.hxx"

namespace Coli
{
	namespace Game
	{
		namespace Objects
		{
			template <bool _Use2D>
			class BasicCamera final :
				public Object,
				public Detail::CameraBase
			{
			public:
				BasicCamera() noexcept = default;

				BasicCamera(BasicCamera&&)	    = delete;
				BasicCamera(BasicCamera const&) = delete;

				BasicCamera& operator=(BasicCamera&&)	   = delete;
				BasicCamera& operator=(BasicCamera const&) = delete;

				void on_start() final {
					myTransform = Object::template make_component <Components::BasicTransform<_Use2D>>();
					Object::on_start();
				}

				void on_late_update (float time) final {
					Object::on_late_update(time);
					hasProjChanged = false;
				}

				_NODISCARD bool has_view_changed() const noexcept {
					if (auto transform = myTransform.lock())
						return transform->has_changed();
					else
						return false;
				}
				
				_NODISCARD glm::mat4 get_view_matrix() const noexcept final 
				{
					const glm::mat4 identity { 1.f };

					if (auto transform = myTransform.lock())
					{
						if constexpr (_Use2D)
						{
							auto const rotator    = glm::angleAxis (glm::radians <float>(transform->rotation)); // glm::conjugate(static_cast <glm::quat>(transform->rotation));
							auto const translator = - glm::vec3{ transform->position, 0.f };

							return glm::mat4_cast (rotator)
								 * glm::translate (identity, translator);
						} 
						else {
							auto const rotator    = glm::conjugate (static_cast <glm::quat>(transform->rotation));
							auto const translator = -static_cast <glm::vec3>(transform->position);

							return glm::mat4_cast (rotator)
								 * glm::translate (identity, translator);
						}
					} 
					
					return identity;
				}

				_NODISCARD glm::mat4 get_projection_matrix() const noexcept final 
				{
					if constexpr (_Use2D)
					{
						auto const height = myFOV / myAspect;

						auto const left   = -myFOV  / 2.0f;
						auto const right  =  myFOV  / 2.0f;
						auto const bottom = -height / 2.0f;
						auto const top    =  height / 2.0f;

						return glm::ortho (left, right, bottom, top, near_clip, far_clip);
					}
					else
						return glm::perspective (glm::radians(myFOV) / myAspect, myAspect, near_clip, far_clip);
				}

				void on_restore(nlohmann::json const& obj) final
				{
					Object::restore(obj);

					using Detail::Json::try_fill;

					float tempFov;
					float tempAspect;

					try_fill (obj, tempFov,    Keys::fov);
					try_fill (obj, tempAspect, Keys::aspect);

					hasProjChanged = true;

					myFOV      = tempFov;
					myAspect   = tempAspect;
				}

				_NODISCARD nlohmann::json on_save() const final
				{
					auto object = Object::save();

					object [Keys::fov]    = myFOV;
					object [Keys::aspect] = myAspect;

					return object;
				}

			private:
				struct Keys {
					static constexpr std::string_view fov    = "fov";
					static constexpr std::string_view aspect = "aspect";
				};

				std::weak_ptr <Components::BasicTransform <_Use2D> const> myTransform;

				using Detail::CameraBase::myAspect;
				using Detail::CameraBase::myFOV;

				static constexpr float near_clip = _Use2D ? -std::numeric_limits <float>::max() : 0.01f;
				static constexpr float far_clip  = _Use2D ?  std::numeric_limits <float>::max() : 10000.f;
			};

			using Camera   = BasicCamera <false>;
			using Camera2D = BasicCamera <true>;
		}
	}
}
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
			template <std::floating_point _FloatTy>
			class Camera final :
				public Object,
				public Detail::CameraBase
			{
			public:
				Camera() noexcept :
					myTransform (Object::make_component<Components::Transform<_FloatTy>>())
				{}

				void on_late_update(float time) final {
					Object::on_late_update(time);
					myProjChanged = false;
				}

				void update_aspect(float aspect) noexcept {
					myAspect      = aspect;
					myProjChanged = true;
				}

				_NODISCARD bool has_view_changed() const noexcept {
					if (auto transform = myTransform.lock())
						return transform->has_changed();
					else
						return false;
				}

				_NODISCARD bool has_proj_changed() const noexcept {
					return myProjChanged;
				}

				_NODISCARD glm::mat4 get_view_matrix() const noexcept 
				{
					const glm::mat4 identity{ 1 };

					if (auto transform = myTransform.lock())
						return glm::mat4_cast(glm::conjugate(static_cast<glm::quat>(transform->rotation)))
							 * glm::translate(identity, -static_cast<glm::vec3>(transform->position));
					else
						return identity;
				}

				_NODISCARD glm::mat4 get_projection_matrix() const noexcept {
					return glm::perspective(
						static_cast<float>(myFOV),
						static_cast<float>(myAspect),
						static_cast<float>(myNearClip),
						static_cast<float>(myFarClip)
					);
				}

				_NODISCARD nlohmann::json serialize() const final 
				{
					auto obj = Object::serialize();

					obj[key_fov]       = myFOV;
					obj[key_aspect]	   = myAspect;
					obj[key_near_clip] = myNearClip;
					obj[key_far_clip]  = myFarClip;

					return obj;
				}

				void deserialize(nlohmann::json const& obj) final
				{
					using Detail::Json::try_fill;

					Object::deserialize(obj);

					float tempFov;
					float tempAspect;
					float tempNearClip;
					float tempFarClip;

					try_fill (obj, tempFov,		 key_fov);
					try_fill (obj, tempAspect,	 key_aspect);
					try_fill (obj, tempNearClip, key_near_clip);
					try_fill (obj, tempFarClip,  key_far_clip);

					myProjChanged = true;

					myFOV	   = tempFov;
					myAspect   = tempAspect;
					myNearClip = tempNearClip;
					myFarClip  = tempFarClip;
				}

			private:
				static constexpr std::string_view key_fov       = "fov";
				static constexpr std::string_view key_aspect    = "aspect";
				static constexpr std::string_view key_near_clip = "nearClip";
				static constexpr std::string_view key_far_clip  = "farClip";

				std::weak_ptr<Components::Transform<_FloatTy> const> myTransform;

				float myFOV    = glm::radians(70.f);
				float myAspect = 1.f;

				float myNearClip = 0.01f;
				float myFarClip  = 10000.f;

				bool myProjChanged = true;
			};
		}
	}
}
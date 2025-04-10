#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "GlmHelper.hxx"
#include "Collider.hxx"

namespace Coli
{
	namespace Detail
	{
		class PolymorphicPhysicalBody
		{
		public:
			virtual void report_collision(PolymorphicCollision const& collision, PolymorphicPhysicalBody& other) {}
			virtual void report_collision(PolymorphicCollision const& collision) {}
		};
	}

	namespace Geometry
	{
		template <std::floating_point _FloatTy, bool _Use2D>
		class BasicPhysicalBody :
			public virtual Detail::PolymorphicPhysicalBody
		{
			void apply_position_correction(glm::vec<_Use2D ? 2 : 3, _FloatTy> const& diff) noexcept {
				if (auto transform = myTransform.lock()) _LIKELY
					transform->position += diff;
			}

		public:
			void report_force(glm::vec<_Use2D ? 2 : 3, _FloatTy> const& direction, _FloatTy magnitude)
			{
				myForcesAccumulator += magnitude / mass * direction;
			}

			void apply_force(glm::vec<_Use2D ? 2 : 3, _FloatTy> const& direction, _FloatTy magnitude, float time = 1) noexcept
			{
				myVelocity += time * magnitude / mass * direction;

				if (myMaxVelocity)
					myVelocity = glm::clamp(myVelocity, -*myMaxVelocity, *myMaxVelocity);
			}

			void apply_forces(float time) noexcept
			{
				myVelocity.y -= time * mass * gravity;
				
				myVelocity -= time / (1 - Detail::clamp_0_1(movingResistance)) * myVelocity;
				myVelocity += time / mass * myForcesAccumulator;

				if (myMaxVelocity)
					myVelocity = glm::clamp(myVelocity, -*myMaxVelocity, *myMaxVelocity);

				myForcesAccumulator = glm::vec<_Use2D ? 2 : 3, _FloatTy>{ 0 };
			}

			void apply_velocity(float time) noexcept {
				if (auto transform = myTransform.lock()) _LIKELY
					transform->position += myVelocity * static_cast<_FloatTy>(time);
			}

			void limit_velocity(glm::vec<_Use2D ? 2 : 3, _FloatTy> const& max) noexcept {
				myMaxVelocity.emplace(glm::abs(max));
			}

			void unleash_velocity() noexcept {
				myMaxVelocity.reset();
			}

			void report_collision(
				Detail::PolymorphicCollision const& collision,
				Detail::PolymorphicPhysicalBody& other
			) final 
			{
				auto const otherBody    = dynamic_cast<BasicPhysicalBody*>	       (std::addressof(other));
				auto const collisionPtr = dynamic_cast<Collision<_FloatTy> const*> (std::addressof(collision));

				if (collisionPtr && otherBody) _LIKELY
				{
					auto const relativeVelocity = myVelocity - otherBody->myVelocity;
					auto const direction		= collisionPtr->direction;

					auto const product = glm::dot(relativeVelocity, direction);

					if (product < 0) _UNLIKELY
					{
						auto const restFactor = -1 - collideRestitution * otherBody->collideRestitution;
						auto const massFactor = 1 / mass + 1 / otherBody->mass;

						auto const impulse = glm::dot(restFactor * relativeVelocity, collisionPtr->normal) / massFactor;

						if (impulse != 0) _LIKELY {
							this->      apply_force(collisionPtr->normal,  impulse);
							otherBody-> apply_force(collisionPtr->normal, -impulse);
						}
					}
					else if (product > 0) 
					{
						auto const correction    = collisionPtr->overlap / (mass + otherBody->mass) * collisionPtr->normal;
						auto const directionSign = -glm::sign(glm::dot(myVelocity, collisionPtr->normal));

						this->      apply_position_correction(correction * (otherBody->mass * -directionSign));
						otherBody-> apply_position_correction(correction * (mass * directionSign));
					}
				}
			}

			void report_collision(Detail::PolymorphicCollision const& collision) noexcept final
			{
				auto const collisionPtr = dynamic_cast<Collision<_FloatTy> const*>(std::addressof(collision));

				if (collisionPtr) {
					auto const reflectedVelocity = (-1 - collideRestitution) * myVelocity;
					this->apply_force(collisionPtr->normal, glm::dot(reflectedVelocity, collisionPtr->normal) * mass);
				}
			}

			void bind_transform(std::weak_ptr<Geometry::BasicTransform<_FloatTy, _Use2D>> transform) noexcept {
				myTransform.swap(transform);
			}

			friend struct nlohmann::adl_serializer <BasicPhysicalBody>;

		protected:
			_NODISCARD bool has_transform() const noexcept {
				return !myTransform.expired();
			}

		private:
			std::weak_ptr<Geometry::BasicTransform<_FloatTy, _Use2D>> myTransform;

			std::optional<glm::vec<_Use2D ? 2 : 3, _FloatTy>> myMaxVelocity;
			glm::vec<_Use2D ? 2 : 3, _FloatTy>				  myForcesAccumulator { 0 };

			glm::vec<_Use2D ? 2 : 3, _FloatTy> myVelocity { 0 };

		public:
			_FloatTy collideRestitution = 0.8;
			_FloatTy movingResistance   = 0.075;
			_FloatTy mass				= 1;
			_FloatTy gravity			= 10;
		};

		template <std::floating_point _FloatTy>
		using PhysicalBody = BasicPhysicalBody<_FloatTy, false>;

		template <std::floating_point _FloatTy>
		using PhysicalBody2D = BasicPhysicalBody<_FloatTy, true>;
	}
}

namespace nlohmann
{
	template <std::floating_point _FloatTy, bool _Use2D>
	struct adl_serializer <Coli::Geometry::BasicPhysicalBody<_FloatTy, _Use2D>>
	{
	private:
		static constexpr std::string_view name_velocity     = "velocity";
		static constexpr std::string_view name_max_velocity = "maxVelocity";

		static constexpr std::string_view name_mass			 = "mass";
		static constexpr std::string_view name_gravity		 = "gravity";
		static constexpr std::string_view name_collide_rest  = "collideRestitution";
		static constexpr std::string_view name_moving_resist = "movingResistance";

	public:
		static void to_json(json& j, Coli::Geometry::BasicPhysicalBody<_FloatTy, _Use2D> const& val)
		{
			j[name_velocity]     = val.myVelocity;
			j[name_max_velocity] = val.myMaxVelocity;

			j[name_mass]		  = val.mass;
			j[name_gravity]		  = val.gravity;
			j[name_collide_rest]  = val.collideRestitution;
			j[name_moving_resist] = val.movingResistance;
		}

		static void from_json(const json& j, Coli::Geometry::BasicPhysicalBody<_FloatTy, _Use2D>& val)
		{
			using Coli::Detail::Json::try_fill;

			decltype (val.myVelocity) tempVelocity;
			try_fill (j, tempVelocity, name_velocity);

			decltype (val.myMaxVelocity) tempMaxVelocity;
			try_fill (j, tempMaxVelocity, name_max_velocity);

			decltype (val.mass) tempMass;
			try_fill (j, tempMass, name_mass);

			decltype (val.gravity) tempGravity;
			try_fill (j, tempGravity, name_gravity);

			decltype (val.collideRestitution) tempCollideRestitution;
			try_fill (j, tempCollideRestitution, name_collide_rest);

			decltype (val.movingResistance) tempMovingResistance;
			try_fill (j, tempMovingResistance, name_moving_resist);

			val.myVelocity    = tempVelocity;
			val.myMaxVelocity = tempMaxVelocity;

			val.mass				 = tempMass;
			val.gravity			 = tempGravity;
			val.collideRestitution = tempCollideRestitution;
			val.movingResistance   = tempMovingResistance;
		}
	};
}
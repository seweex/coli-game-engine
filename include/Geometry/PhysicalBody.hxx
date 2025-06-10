#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "GlmHelper.hxx"
#include "Collider.hxx"

namespace Coli
{
	namespace Geometry
	{
		template <bool _Use2D>
		class BasicPhysicalBody
		{
			using vector_type = glm::vec <_Use2D ? 2 : 3, double>;

			void apply_position_correction (vector_type const& diff) noexcept {
				if (auto transform = myTransform.lock())
					transform->position += diff;
			}

		public:
			BasicPhysicalBody() noexcept = default;

			BasicPhysicalBody(BasicPhysicalBody const& other) noexcept {
				*this = other;
			}

			BasicPhysicalBody& operator=(BasicPhysicalBody const& other) noexcept
			{
				myMaxVelocity		= other.myMaxVelocity;
				myForcesAccumulator = other.myForcesAccumulator;
				myVelocity			= other.myVelocity;

				collideRestitution = other.collideRestitution;
				movingResistance   = other.movingResistance;
				mass			   = other.mass;
				gravity			   = other.gravity;

				return *this;
			}

			void report_force (vector_type const& direction, double magnitude) noexcept {
				myForcesAccumulator += magnitude / mass * direction;
			}

			void report_force (vector_type const& force) noexcept {
				myForcesAccumulator += force / mass;
			}

			void apply_force (vector_type const& direction, double magnitude, float time) noexcept
			{
				myVelocity += time * magnitude / mass * direction;

				if (myMaxVelocity.has_value())
					myVelocity = glm::clamp(myVelocity, -*myMaxVelocity, *myMaxVelocity);
			}

			void apply_forces (float time) noexcept
			{
				myVelocity.y -= time * mass * gravity;
				
				myVelocity -= time / (1.0 - glm::clamp (movingResistance, 0.0, 1.0)) * myVelocity;
				myVelocity += time / mass * myForcesAccumulator;

				if (myMaxVelocity.has_value())
					myVelocity = glm::clamp (myVelocity, -*myMaxVelocity, *myMaxVelocity);

				myForcesAccumulator = vector_type{ 0 };
			}

			void apply_velocity (float time) noexcept {
				if (auto transform = myTransform.lock())
					transform->position += myVelocity * static_cast<double>(time);
			}

			void limit_velocity (vector_type const& max) noexcept {
				myMaxVelocity.emplace (glm::abs(max));
			}

			void unleash_velocity() noexcept {
				myMaxVelocity.reset();
			}

			void report_collision(
				BasicPhysicalBody const& other,
				BasicCollision<_Use2D> const& collision
			) noexcept 
			{
				auto const relativeVelocity = myVelocity - other.myVelocity;
				auto const direction		= collision.direction;

				auto const product = glm::dot(relativeVelocity, direction);

				if (product > 0 && false)
				{
					auto const correction    = collision.overlap / (mass + other.mass) * collision.normal;
					auto const directionSign = glm::sign (glm::dot(myVelocity, collision.normal));

					this->apply_position_correction (correction * (other.mass * directionSign));
					other.apply_position_correction (correction * (mass * -directionSign));
				}
				else if (product < 0)
				{
					auto myRestitution    = glm::clamp (collideRestitution, 0.0, 1.0);
					auto otherRestitution = glm::clamp (other.collideRestitution, 0.0, 1.0);

					auto const restFactor = -1 - myRestitution * otherRestitution;
					auto const massFactor = 1 / mass + 1 / other.mass;

					auto const impulse = glm::dot(restFactor * relativeVelocity, collision.normal) / massFactor;

					if (impulse != 0) {
						this->apply_force (collision.normal,  impulse);
						other.apply_force (collision.normal, -impulse);
					}
				}
			}

			void report_collision(BasicCollision<_Use2D> const& collision) noexcept 
			{
				auto const restitution       = glm::clamp(collideRestitution, 0.0, 1.0);
				auto const reflectedVelocity = (-1 - restitution) * myVelocity;
				auto const force			 = glm::dot(reflectedVelocity, collision.normal) * mass;

				this->apply_force(collision.normal, force);
			}

			void bind_transform(std::weak_ptr <Geometry::BasicTransform <_Use2D>> transform) noexcept {
				myTransform.swap(transform);
			}

			friend struct nlohmann::adl_serializer <BasicPhysicalBody>;

		protected:
			_NODISCARD bool has_transform() const noexcept {
				return !myTransform.expired();
			}

		private:
			std::weak_ptr <Geometry::BasicTransform <_Use2D>> myTransform;

			std::optional <vector_type> myMaxVelocity;

			vector_type myForcesAccumulator { 0.0 };
			vector_type myVelocity			{ 0.0 };

		public:
			double collideRestitution = 0.8;
			double movingResistance   = 0.075;
			double mass				  = 1;
			double gravity			  = 10;
		};
		
		using PhysicalBody   = BasicPhysicalBody <false>;
		using PhysicalBody2D = BasicPhysicalBody <true>;
	}
}

namespace nlohmann
{
	template <bool _Use2D>
	struct adl_serializer <Coli::Geometry::BasicPhysicalBody<_Use2D>>
	{
	private:
		struct Keys {
			static constexpr std::string_view velocity	   = "velocity";
			static constexpr std::string_view max_velocity = "maxVelocity";

			static constexpr std::string_view mass			= "mass";
			static constexpr std::string_view gravity       = "gravity";
			static constexpr std::string_view collide_rest  = "collideRestitution";
			static constexpr std::string_view moving_resist = "movingResistance";
		};

	public:
		static void to_json(json& j, Coli::Geometry::BasicPhysicalBody<_Use2D> const& val)
		{
			j [Keys::velocity]     = val.myVelocity;
			j [Keys::max_velocity] = val.myMaxVelocity;

			j [Keys::mass]		    = val.mass;
			j [Keys::gravity]	    = val.gravity;
			j [Keys::collide_rest]  = val.collideRestitution;
			j [Keys::moving_resist] = val.movingResistance;
		}

		static void from_json (const json& j, Coli::Geometry::BasicPhysicalBody<_Use2D>& val)
		{
			using Coli::Detail::Json::try_fill;

			decltype (val.myVelocity) tempVelocity;
			try_fill (j, tempVelocity, Keys::velocity);

			decltype (val.myMaxVelocity) tempMaxVelocity;
			try_fill (j, tempMaxVelocity, Keys::max_velocity);

			decltype (val.mass) tempMass;
			try_fill (j, tempMass, Keys::mass);

			decltype (val.gravity) tempGravity;
			try_fill (j, tempGravity, Keys::gravity);

			decltype (val.collideRestitution) tempCollideRestitution;
			try_fill (j, tempCollideRestitution, Keys::collide_rest);

			decltype (val.movingResistance) tempMovingResistance;
			try_fill (j, tempMovingResistance, Keys::moving_resist);

			val.myVelocity    = tempVelocity;
			val.myMaxVelocity = tempMaxVelocity;

			val.mass			   = tempMass;
			val.gravity			   = tempGravity;
			val.collideRestitution = tempCollideRestitution;
			val.movingResistance   = tempMovingResistance;
		}
	};
}
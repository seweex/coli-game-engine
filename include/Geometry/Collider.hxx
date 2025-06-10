#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Transform.hxx"

namespace Coli
{
	namespace Geometry
	{
		template <bool _Use2D>
		class BasicCollision
		{
			using vector_type = glm::vec <_Use2D ? 2 : 3, double>;

		public:
			constexpr BasicCollision() noexcept = default;
	
			vector_type direction;
			vector_type normal;
			double		overlap;
		};

		using Collision   = BasicCollision <false>;
		using Collision2D = BasicCollision <true>;
	}

	namespace Detail
	{
		inline namespace SAT
		{
			template <bool _Use2D>
			class ColliderBase
			{
				using vector_type = glm::vec <_Use2D ? 2 : 3, double>;

			public:
				constexpr ColliderBase() noexcept = default;

				constexpr virtual ~ColliderBase() noexcept = default;

				ColliderBase(ColliderBase&&)	  noexcept {}
				ColliderBase(ColliderBase const&) noexcept {}

				ColliderBase& operator=(ColliderBase&&)		 noexcept { return *this; }
				ColliderBase& operator=(ColliderBase const&) noexcept { return *this; }

			protected:
				_NODISCARD vector_type get_world_position() const noexcept
				{
					if (auto transform = myTransform.lock())
						return transform->get_world_position();

					return vector_type{ 0 };
				}

				_NODISCARD virtual double get_longest_diagonal() const noexcept = 0;

				_NODISCARD virtual std::unordered_set <vector_type>
				get_axes() const = 0;

				_NODISCARD virtual std::pair <double, double>
				get_projection(vector_type const& axis) const noexcept = 0;
				
			private:
				_NODISCARD static double find_overlap(
					std::pair <double, double> const& left,
					std::pair <double, double> const& right
				) noexcept
				{
					auto const [leftStart,  leftEnd]  = std::minmax(left.first,  left.second);
					auto const [rightStart, rightEnd] = std::minmax(right.first, right.second);

					if (leftEnd <= rightStart || rightEnd <= leftStart)
						return 0;

					auto const overlapStart = std::max(leftStart, rightStart);
					auto const overlapEnd   = std::min(leftEnd,   rightEnd);

					return overlapEnd - overlapStart;
				}

			public:
				_NODISCARD static std::optional <Geometry::BasicCollision<_Use2D>>
				find_collision (
					ColliderBase const& first,
					ColliderBase const& second
				) {
					auto const distance    = first.get_world_position() - second.get_world_position();
					auto const maxDiagonal = first.get_longest_diagonal() + second.get_longest_diagonal();

					if (glm::length2(distance) <= maxDiagonal * maxDiagonal)
					{
						auto	   axes      = first.get_axes();
						auto const otherAxes = second.get_axes();

						axes.insert(otherAxes.begin(), otherAxes.end());

						if (axes.empty())
							axes.emplace (glm::normalize(distance));

						vector_type normal;
						double minOverlap = std::numeric_limits<double>::infinity();

						for (auto const& axis : axes)
						{
							auto const overlap = find_overlap(first.get_projection(axis), second.get_projection(axis));
							
							if (overlap > 0) {
								if (overlap < minOverlap) {
									minOverlap = overlap;
									normal = axis;
								}
							}
							else
								return std::nullopt;
						}

						return Geometry::BasicCollision<_Use2D>{ distance, normal, minOverlap };
					} 
					else
						return std::nullopt;
				}

				_NODISCARD std::optional <Geometry::BasicCollision<_Use2D>> 
				find_collision (ColliderBase const& other) {
					return find_collision(*this, other);
				}

				void bind_transform(std::weak_ptr<Geometry::BasicTransform<_Use2D> const> transform) noexcept {
					myTransform.swap(transform);
				}

			protected:
				_NODISCARD bool has_transform() const noexcept {
					return !myTransform.expired();
				}

				std::weak_ptr <Geometry::BasicTransform <_Use2D> const> myTransform;
			};
		}
	}

	namespace Geometry
	{
		inline namespace SAT
		{
			template <bool _Use2D>
			class BasicBoxCollider :
				public Detail::SAT::ColliderBase<_Use2D>
			{
				using vector_type  = glm::vec <_Use2D ? 2 : 3, double>;
				using rotator_type = std::conditional_t <_Use2D, double, glm::dquat>;

			public:
				BasicBoxCollider () noexcept = default;

				BasicBoxCollider (
					vector_type const& size,
					rotator_type const& rotator
				) noexcept :
					myHalfSizes (size / 2.0),
					myRotation  (rotator),
					myDiagonal  (glm::length (myHalfSizes))
				{}

				BasicBoxCollider(BasicBoxCollider&&)	  noexcept = default;
				BasicBoxCollider(BasicBoxCollider const&) noexcept = default;

				BasicBoxCollider& operator=(BasicBoxCollider&)		 noexcept = default;
				BasicBoxCollider& operator=(BasicBoxCollider const&) noexcept = default;

				void disable_transform_rotation() noexcept {
					myIgnoreRotationFlag = true;
				}

				void enable_transform_rotation() noexcept {
					myIgnoreRotationFlag = false;
				}

			private:
				_NODISCARD rotator_type get_rotatator() const noexcept
				{
					if (myIgnoreRotationFlag)
						return myRotation;

					else if (auto transform = myTransform.lock()) {
						if constexpr (_Use2D)
							return myRotation + transform->get_world_rotation();
						else
							return myRotation * transform->get_world_rotation();
					}
					else
						return myRotation;
				}

				_NODISCARD double get_longest_diagonal() const noexcept final {
					return myDiagonal;
				}
				
				_NODISCARD std::unordered_set <vector_type>
				get_axes() const final 
				{
					auto const rotator = get_rotatator();

					if constexpr (_Use2D)
						return {
							glm::dvec2{ glm::cos(glm::radians(rotator)), 0 },
							glm::dvec2{ 0, glm::sin(glm::radians(rotator)) }
						};
					else
						return {
							glm::rotate (rotator, glm::dvec3{ 1, 0, 0 }),
							glm::rotate (rotator, glm::dvec3{ 0, 1, 0 }),
							glm::rotate (rotator, glm::dvec3{ 0, 0, 1 })
						};
				}

				_NODISCARD std::pair <double, double> 
				get_projection (vector_type const& axis) const noexcept final
				{
					auto const projCenter = glm::dot(this->get_world_position(), axis);
					auto const rotator    = get_rotatator();
					auto	   boxAxes    = myHalfSizes;

					if (auto transform = myTransform.lock())
						boxAxes *= transform->get_world_scale();

					if constexpr (_Use2D) {
						boxAxes *= glm::dvec2{ glm::cos(glm::radians(rotator)), glm::sin(glm::radians(rotator)) };
						boxAxes =  glm::abs(boxAxes);

						return {
							projCenter - boxAxes.x - boxAxes.y,
							projCenter + boxAxes.x + boxAxes.y
						};
					} 
					else {
						boxAxes = glm::rotate (rotator, boxAxes);
						boxAxes = glm::abs (glm::dvec3 {
							glm::dot (glm::dvec3{ boxAxes.x, 0, 0 }, axis),
							glm::dot (glm::dvec3{ 0, boxAxes.y, 0 }, axis),
							glm::dot (glm::dvec3{ 0, 0, boxAxes.z }, axis)
						});

						return {
							projCenter - boxAxes.x - boxAxes.y - boxAxes.z,
							projCenter + boxAxes.x + boxAxes.y + boxAxes.z
						};
					}
				}

				friend struct nlohmann::adl_serializer <BasicBoxCollider>;

				using Detail::SAT::ColliderBase <_Use2D>::myTransform;

				rotator_type myRotation;
				vector_type  myHalfSizes;
				double	     myDiagonal;
				bool		 myIgnoreRotationFlag;
			};

			template <bool _Use2D>
			class BasicRoundCollider :
				public Detail::SAT::ColliderBase <_Use2D>
			{
				using vector_type = glm::vec <_Use2D ? 2 : 3, double>;

			public:
				BasicRoundCollider() noexcept = default;

				BasicRoundCollider (double radius) noexcept :
					myRadius (radius)
				{}

				BasicRoundCollider(BasicRoundCollider&&)	  noexcept = default;
				BasicRoundCollider(BasicRoundCollider const&) noexcept = default;

				BasicRoundCollider& operator=(BasicRoundCollider &)		 noexcept = default;
				BasicRoundCollider& operator=(BasicRoundCollider const&) noexcept = default;

			private:
				_NODISCARD double get_longest_diagonal() const noexcept final {
					return myRadius;
				}

				_NODISCARD std::unordered_set <vector_type>
				get_axes() const final {
					return {};
				}

				_NODISCARD std::pair <double, double>
				get_projection (vector_type const& axis) const noexcept final
				{
					auto const projCenter = glm::dot(this->get_world_position(), axis);

					return { 
						projCenter - myRadius,
						projCenter + myRadius 
					};
				}

				friend struct nlohmann::adl_serializer <BasicRoundCollider>;

				double myRadius;
			};

			using BoxCollider   = BasicBoxCollider <false>;
			using BoxCollider2D = BasicBoxCollider <true>;

			using SphereCollider   = BasicRoundCollider <false>;
			using CircleCollider2D = BasicRoundCollider <true>;
		}
	}
}

namespace nlohmann
{
	template <bool _Use2D>
	struct adl_serializer <Coli::Geometry::BasicBoxCollider<_Use2D>>
	{
	private:
		struct Keys {
			static constexpr std::string_view half_sizes	  = "halfSizes";
			static constexpr std::string_view rotation		  = "rotation";
			static constexpr std::string_view ignore_rotation = "ignoreRotation";
			static constexpr std::string_view diagonal		  = "diagonal";
		};

	public:
		static void to_json(json& j, Coli::Geometry::BasicBoxCollider <_Use2D> const& val)
		{
			j [Keys::half_sizes]	  = val.myHalfSizes;
			j [Keys::rotation]		  = val.myRotation;
			j [Keys::ignore_rotation] = val.myIgnoreRotationFlag;
			j [Keys::diagonal]		  = val.myDiagonal;
		}

		static void from_json(const json& j, Coli::Geometry::BasicBoxCollider <_Use2D>& val)
		{
			using Coli::Detail::Json::try_fill;

			decltype (val.myHalfSizes) tempHalfSizes;
			try_fill (j, tempHalfSizes, Keys::half_sizes);

			decltype (val.myRotation) tempRotation;
			try_fill (j, tempRotation, Keys::rotation);

			decltype (val.myIgnoreRotationFlag) tempIgnoreRotation;
			try_fill(j, tempIgnoreRotation, Keys::ignore_rotation);

			decltype (val.myDiagonal) tempDiagonal;
			try_fill (j, tempDiagonal, Keys::diagonal);

			val.myHalfSizes			 = tempHalfSizes;
			val.myRotation			 = tempRotation;
			val.myIgnoreRotationFlag = tempIgnoreRotation;
			val.myDiagonal			 = tempDiagonal;
		}
	};

	template <bool _Use2D>
	struct adl_serializer <Coli::Geometry::BasicRoundCollider<_Use2D>>
	{
	private:
		struct Keys {
			static constexpr std::string_view radius = "radius";
		};

	public:
		static void to_json(json& j, Coli::Geometry::BasicRoundCollider <_Use2D> const& val) {
			j [Keys::radius] = val.myRadius;
		}

		static void from_json(const json& j, Coli::Geometry::BasicRoundCollider <_Use2D>& val) {
			Coli::Detail::Json::try_fill (j, val.myRadius, Keys::radius);
		}
	};
}
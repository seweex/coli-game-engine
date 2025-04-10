#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Transform.hxx"

namespace Coli
{
	namespace Detail
	{
		class PolymorphicCollision
		{
		protected:
			constexpr PolymorphicCollision() noexcept = default;

		public:
			constexpr virtual ~PolymorphicCollision() noexcept = default;
		};
	}

	namespace Geometry
	{
		template <std::floating_point _FloatTy, bool _Use2D>
		class BasicCollision :
			public Detail::PolymorphicCollision
		{
		public:
			constexpr BasicCollision(
				glm::vec<_Use2D ? 2 : 3, _FloatTy> const& dir,
				glm::vec<_Use2D ? 2 : 3, _FloatTy> const& norm,
				_FloatTy over
			) noexcept :
				direction (dir),
				normal    (norm),
				overlap   (over)
			{}
	
			glm::vec<_Use2D ? 2 : 3, _FloatTy> direction;
			glm::vec<_Use2D ? 2 : 3, _FloatTy> normal;
			_FloatTy overlap;
		};

		template <std::floating_point _FloatTy>
		using Collision = BasicCollision<_FloatTy, false>;

		template <std::floating_point _FloatTy>
		using Collision2D = BasicCollision<_FloatTy, true>;
	}

	namespace Detail
	{
		inline namespace SAT
		{
			class PolymorphicCollider
			{
			protected:
				constexpr PolymorphicCollider() noexcept = default;

			public:
				_NODISCARD virtual std::unique_ptr<PolymorphicCollision> find_collision(PolymorphicCollider const& other) {
					return nullptr;
				}
			};

			template <std::floating_point _FloatTy, bool _Use2D>
			class ColliderBase :
				public virtual PolymorphicCollider
			{
			protected:
				_NODISCARD glm::vec<_Use2D ? 2 : 3, _FloatTy> get_world_position() const noexcept
				{
					if (auto transform = myTransform.lock()) _LIKELY
						return transform->get_world_position();

					return glm::vec<_Use2D ? 2 : 3, _FloatTy>{ 0 };
				}

				_NODISCARD virtual _FloatTy get_longest_diagonal() const noexcept = 0;

				_NODISCARD virtual std::unordered_set<glm::vec<_Use2D ? 2 : 3, _FloatTy>>
				get_axes() const = 0;

				_NODISCARD virtual std::pair<_FloatTy, _FloatTy>
				get_projection(glm::vec<_Use2D ? 2 : 3, _FloatTy> const& axis) const noexcept = 0;				
				
			private:
				_NODISCARD static std::optional<_FloatTy> find_overlap(
					std::pair<_FloatTy, _FloatTy> const& left,
					std::pair<_FloatTy, _FloatTy> const& right
				) noexcept
				{
					auto const [leftStart,  leftEnd]  = std::minmax(left.first,  left.second);
					auto const [rightStart, rightEnd] = std::minmax(right.first, right.second);

					if (leftEnd <= rightStart || rightEnd <= leftStart)
						return std::nullopt;

					auto const overlapStart = std::max(leftStart, rightStart);
					auto const overlapEnd   = std::min(leftEnd,   rightEnd);

					return overlapEnd - overlapStart;
				}

			public:
				_NODISCARD static std::optional<Geometry::BasicCollision<_FloatTy, _Use2D>>
				find_collision(
					ColliderBase const& first,
					ColliderBase const& second
				) {
					auto const distance    = first.get_world_position()   - second.get_world_position();
					auto const maxDiagonal = first.get_longest_diagonal() + second.get_longest_diagonal();

					if (glm::length2(distance) <= (maxDiagonal * maxDiagonal))
					{
						auto	   axes      = first.  get_axes();
						auto const otherAxes = second. get_axes();

						axes.insert(otherAxes.begin(), otherAxes.end());

						if (axes.empty())
							axes.emplace (glm::normalize(distance));

						glm::vec<_Use2D ? 2 : 3, _FloatTy> normal;
						_FloatTy minOverlap = std::numeric_limits<_FloatTy>::infinity();

						for (auto const& axis : axes)
							if (auto const overlap = find_overlap(first.  get_projection(axis), 
															      second. get_projection(axis))
							) { 
								if (*overlap < minOverlap) {
									minOverlap = *overlap;
									normal	   = axis;
								}
							} else
								return std::nullopt;

						return Geometry::BasicCollision<_FloatTy, _Use2D>(distance, glm::abs(normal), minOverlap);
					} 
					else
						return std::nullopt;
				}

				_NODISCARD std::unique_ptr<Detail::PolymorphicCollision> find_collision(PolymorphicCollider const& other) final
				{
					if (auto otherPtr = dynamic_cast<ColliderBase const*>(std::addressof(other)))
						if (auto collision = find_collision(*this, *otherPtr))
							return std::make_unique<Geometry::BasicCollision<_FloatTy, _Use2D>>(*collision);
					
					return nullptr;
				}

				void bind_transform(std::weak_ptr<Geometry::BasicTransform<_FloatTy, _Use2D> const> transform) noexcept {
					myTransform.swap(transform);
				}

			protected:
				_NODISCARD bool has_transform() const noexcept {
					return !myTransform.expired();
				}

				std::weak_ptr<Geometry::BasicTransform<_FloatTy, _Use2D> const> myTransform;
			};
		}
	}

	namespace Geometry
	{
		inline namespace SAT
		{
			template <std::floating_point _FloatTy, bool _Use2D>
			class BasicBoxCollider :
				public Detail::SAT::ColliderBase<_FloatTy, _Use2D>
			{
				using rotator_type = std::conditional_t<_Use2D, _FloatTy, glm::qua<_FloatTy>>;

			public:
				BasicBoxCollider(
					glm::vec <_Use2D ? 2 : 3, _FloatTy> const& size = glm::vec<_Use2D ? 2 : 3, _FloatTy>{ 1 },
					rotator_type const& rotator = {}
				) noexcept :
					myHalfSizes (size / static_cast<_FloatTy>(2)),
					myRotation  (rotator),
					myDiagonal  (glm::length(myHalfSizes))
				{}

				void disable_transform_rotation() noexcept {
					myIgnoreRotationFlag = true;
				}

				void enable_transform_rotation() noexcept {
					myIgnoreRotationFlag = false;
				}

			private:
				_NODISCARD rotator_type get_rotatator() const noexcept
				{
					auto transform = myTransform.lock();

					if (!myIgnoreRotationFlag && transform) _LIKELY
					{
						if constexpr (_Use2D)
							return myRotation + transform->get_world_rotation();
						else
							return myRotation * transform->get_world_rotation();
					}					
					else
						return myRotation;
				}

				_NODISCARD _FloatTy get_longest_diagonal() const noexcept final {
					return myDiagonal;
				}
				
				_NODISCARD std::unordered_set<glm::vec<_Use2D ? 2 : 3, _FloatTy>>
				get_axes() const final 
				{
					auto const rotator = get_rotatator();

					if constexpr (_Use2D)
						return {
							glm::vec<2, _FloatTy>{ glm::cos(rotator), 0 },
							glm::vec<2, _FloatTy>{ 0, glm::sin(rotator) }
						};
					else
						return {
							glm::rotate(rotator, glm::vec<3, _FloatTy>{ 1, 0, 0 }),
							glm::rotate(rotator, glm::vec<3, _FloatTy>{ 0, 1, 0 }),
							glm::rotate(rotator, glm::vec<3, _FloatTy>{ 0, 0, 1 })
						};
				}

				_NODISCARD std::pair<_FloatTy, _FloatTy> 
				get_projection(glm::vec<_Use2D ? 2 : 3, _FloatTy> const& axis) const noexcept final
				{
					auto const projCenter = glm::dot(this->get_world_position(), axis);
					auto const rotator    = get_rotatator();
					auto	   boxAxes    = myHalfSizes;

					if (auto transform = myTransform.lock()) _LIKELY
						boxAxes *= transform->get_world_scale();

					if constexpr (_Use2D)
					{
						boxAxes *= glm::vec<2, _FloatTy>(glm::cos(rotator), glm::sin(rotator));
						boxAxes =  glm::abs(boxAxes);

						return {
							projCenter - boxAxes.x - boxAxes.y,
							projCenter + boxAxes.x + boxAxes.y
						};
					}
					else
					{
						boxAxes = glm::rotate (rotator, boxAxes);
						boxAxes = glm::abs    (glm::vec<3, _FloatTy> {
							glm::dot(glm::vec<3, _FloatTy>(boxAxes.x, 0, 0), axis),
							glm::dot(glm::vec<3, _FloatTy>(0, boxAxes.y, 0), axis),
							glm::dot(glm::vec<3, _FloatTy>(0, 0, boxAxes.z), axis),
						});

						return {
							projCenter - boxAxes.x - boxAxes.y - boxAxes.z,
							projCenter + boxAxes.x + boxAxes.y + boxAxes.z
						};
					}
				}

				friend struct nlohmann::adl_serializer<BasicBoxCollider>;

				using Detail::SAT::ColliderBase<_FloatTy, _Use2D>::myTransform;

				rotator_type myRotation;
				bool		 myIgnoreRotationFlag;

				glm::vec<_Use2D ? 2 : 3, _FloatTy> myHalfSizes;

				_FloatTy myDiagonal;
			};

			template <std::floating_point _FloatTy, bool _Use2D>
			class BasicRoundCollider :
				public Detail::SAT::ColliderBase<_FloatTy, _Use2D>
			{
			public:
				BasicRoundCollider(_FloatTy radius = 1) noexcept :
					myRadius (radius)
				{}

			private:
				_NODISCARD _FloatTy get_longest_diagonal() const noexcept final {
					return myRadius;
				}

				_NODISCARD std::unordered_set<glm::vec<_Use2D ? 2 : 3, _FloatTy>>
				get_axes() const final 
				{
					return {};
				}

				_NODISCARD std::pair<_FloatTy, _FloatTy>
				get_projection(glm::vec<_Use2D ? 2 : 3, _FloatTy> const& axis) const noexcept final
				{
					auto const projCenter = glm::dot(this->get_world_position(), axis);
					return { projCenter - myRadius, projCenter + myRadius };
				}

				friend struct nlohmann::adl_serializer<BasicRoundCollider>;

			private:
				_FloatTy myRadius;
			};

			template <std::floating_point _FloatTy>
			using BoxCollider = BasicBoxCollider<_FloatTy, false>;
		
			template <std::floating_point _FloatTy>
			using BoxCollider2D = BasicBoxCollider<_FloatTy, true>;

			template <std::floating_point _FloatTy>
			using SphereCollider = BasicRoundCollider<_FloatTy, false>;

			template <std::floating_point _FloatTy>
			using CircleCollider2D = BasicRoundCollider<_FloatTy, true>;
		}
	}
}

namespace nlohmann
{
	template <std::floating_point _FloatTy, bool _Use2D>
	struct adl_serializer <Coli::Geometry::BasicBoxCollider<_FloatTy, _Use2D>>
	{
	private:
		static constexpr std::string_view name_half_sizes	   = "halfSizes";
		static constexpr std::string_view name_rotation		   = "rotation";
		static constexpr std::string_view name_ignore_rotation = "ignoreRotation";
		static constexpr std::string_view name_diagonal		   = "diagonal";

	public:
		static void to_json(json& j, Coli::Geometry::BasicBoxCollider<_FloatTy, _Use2D> const& val)
		{
			j[name_half_sizes]		= val.myHalfSizes;
			j[name_rotation]		= val.myRotation;
			j[name_ignore_rotation] = val.myIgnoreRotationFlag;
			j[name_diagonal]		= val.myDiagonal;
		}

		static void from_json(const json& j, Coli::Geometry::BasicBoxCollider<_FloatTy, _Use2D>& val)
		{
			using Coli::Detail::Json::try_fill;

			decltype (val.myHalfSizes) tempHalfSizes;
			try_fill (j, tempHalfSizes, name_half_sizes);

			decltype (val.myRotation) tempRotation;
			try_fill (j, tempRotation, name_rotation);

			decltype (val.myIgnoreRotationFlag) tempIgnoreRotation;
			try_fill(j, tempIgnoreRotation, name_ignore_rotation);

			decltype (val.myDiagonal) tempDiagonal;
			try_fill (j, tempDiagonal, name_diagonal);

			val.myHalfSizes			 = tempHalfSizes;
			val.myRotation			 = tempRotation;
			val.myIgnoreRotationFlag = tempIgnoreRotation;
			val.myDiagonal			 = tempDiagonal;
		}
	};

	template <std::floating_point _FloatTy, bool _Use2D>
	struct adl_serializer <Coli::Geometry::BasicRoundCollider<_FloatTy, _Use2D>>
	{
	private:
		static constexpr std::string_view name_radius = "radius";

	public:
		static void to_json(json& j, Coli::Geometry::BasicRoundCollider<_FloatTy, _Use2D> const& val) {
			j[name_radius] = val.myRadius;
		}

		static void from_json(const json& j, Coli::Geometry::BasicRoundCollider<_FloatTy, _Use2D>& val) {
			Coli::Detail::Json::try_fill(j, val.myRadius, name_radius);
		}
	};
}
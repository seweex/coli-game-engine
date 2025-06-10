#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "GlmHelper.hxx"

namespace Coli
{
	namespace Geometry
	{
		template <bool _Use2D>
		class BasicTransform :
			public Detail::KeepsChangeBase <BasicTransform<_Use2D>>
		{
			using vector_type  = glm::vec <_Use2D ? 2 : 3, double>;
			using rotator_type = std::conditional_t <_Use2D, double, glm::dquat>;

		public:
			BasicTransform() noexcept = default;

			BasicTransform (BasicTransform const& other) noexcept {
				*this = other;
			}

			BasicTransform& operator=(BasicTransform const& other) noexcept 
			{
				rotation = other.rotation;
				position = other.position;
				scale    = other.scale;

				this->commit(*this);
				return *this;
			}

			_NODISCARD glm::dmat4 get_model_matrix() const noexcept 
			{
				const glm::dmat4 identity { 1 };

				if constexpr (_Use2D)
				{
					const glm::vec3 rotationAxis { 0, 0, 1 };

					return glm::translate (identity, { position, 0 })
						 * glm::rotate    (identity, rotation, rotationAxis)
						 * glm::scale     (identity, { scale, 1 });
				}
				else 
					return glm::translate (identity, position)
						 * glm::mat4_cast (rotation)
						 * glm::scale     (identity, scale);
			}

			void bind_to (std::weak_ptr <BasicTransform const> parent) noexcept {
				myParent.swap(parent);
			}

			_NODISCARD rotator_type get_world_rotation() const noexcept
			{
				if (auto parent = myParent.lock())
				{
					if constexpr (_Use2D)
						return rotation + parent->get_world_rotation();
					else
						return rotation * parent->get_world_rotation();
				}
				else
					return rotation;
			}

			_NODISCARD vector_type get_world_position() const noexcept
			{
				if (auto parent = myParent.lock())
					return parent->get_world_position() + position;
				else
					return position;
			}

			_NODISCARD vector_type get_world_scale() const noexcept
			{
				if (auto parent = myParent.lock())
					return parent->get_world_scale() * scale;
				else
					return scale;
			}

			friend struct nlohmann::adl_serializer <BasicTransform<_Use2D>>;

		private:
			std::weak_ptr <BasicTransform const> myParent;

		public:
			rotator_type rotation = Detail::default_rotator <rotator_type>;

			vector_type position { 0.0 };
			vector_type scale    { 1.0 };
		};

		using Transform   = BasicTransform <false>;
		using Transform2D = BasicTransform <true>;
	}
}

namespace std
{
	template <bool _Use2D>
	struct hash <Coli::Geometry::BasicTransform <_Use2D>>
	{
		_NODISCARD size_t operator()(Coli::Geometry::BasicTransform <_Use2D> const& val) const noexcept 
		{
			Coli::Detail::HashMixer mixer;
			size_t hash;

			hash = mixer(std::hash<decltype(val.position)>{}(val.position));
			hash = mixer(std::hash<decltype(val.rotation)>{}(val.rotation), hash);
			hash = mixer(std::hash<decltype(val.scale)>{}(val.scale), hash);

			return hash;
		}
	};
}

namespace nlohmann
{
	template <bool _Use2D>
	struct adl_serializer <Coli::Geometry::BasicTransform <_Use2D>>
	{
	private:
		struct Keys {
			static constexpr std::string_view position = "position";
			static constexpr std::string_view rotation = "rotation";
			static constexpr std::string_view scale    = "scale";
		};

	public:
		static void to_json(json& j, Coli::Geometry::BasicTransform<_Use2D> const& val)
		{
			j[Keys::position] = val.position;
			j[Keys::rotation] = val.rotation;
			j[Keys::scale]    = val.scale;
		}

		static void from_json(const json& j, Coli::Geometry::BasicTransform<_Use2D>& val)
		{
			using Coli::Detail::Json::try_fill;

			decltype (val.position) tempPosition;
			try_fill (j, tempPosition, Keys::position);

			decltype (val.rotation) tempRotation;
			try_fill (j, tempRotation, Keys::rotation);

			decltype (val.scale) tempScale;
			try_fill (j, tempScale, Keys::scale);

			val.position = tempPosition;
			val.rotation = tempRotation;
			val.scale    = tempScale;
		}
	};
}
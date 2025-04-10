#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

namespace Coli
{
	namespace Geometry
	{
		template <std::floating_point _FloatTy, bool _Is2D>
		class BasicTransform
		{
			using rotator_type = std::conditional_t<_Is2D, _FloatTy, glm::qua<_FloatTy>>;

			_NODISCARD static constexpr rotator_type gen_default_rotation() noexcept 
			{
				if constexpr (_Is2D)
					return 0;
				else
					return glm::qua<_FloatTy>::wxyz(1, 0, 0, 0);
			}

		public:
			_NODISCARD glm::mat<4, 4, _FloatTy> get_model_matrix() const noexcept 
			{
				const glm::mat<4, 4, _FloatTy> identity { 1 };

				if constexpr (_Is2D)
				{
					const glm::vec<3, _FloatTy> rotationAxis{ 0, 0, 1 };

					return glm::translate (identity, { position, 0 })
						 * glm::rotate    (identity, rotation, rotationAxis)
						 * glm::scale     (identity, { scale, 1 });
				}
				else 
					return glm::translate (identity, position)
						 * glm::mat4_cast (rotation)
						 * glm::scale     (identity, scale);
			}

			_NODISCARD bool has_changed() const noexcept {
				return myPrevHash != myCurrentHash;
			}

			void commit_change() noexcept
			{
				myPrevHash = myCurrentHash;
				
				myCurrentHash = Detail::FNV::mix_hash(std::hash<decltype(position)>{}(position));
				myCurrentHash = Detail::FNV::mix_hash(std::hash<decltype(rotation)>{}(rotation), myCurrentHash);
				myCurrentHash = Detail::FNV::mix_hash(std::hash<decltype(scale)>{}(scale), myCurrentHash);
			}

			void bind_to(std::weak_ptr<BasicTransform const> parent) noexcept {
				myParent.swap(parent);
			}

			_NODISCARD rotator_type get_world_rotation() const noexcept
			{
				if (auto parent = myParent.lock()) _UNLIKELY
				{
					if constexpr (_Is2D)
						return rotation + parent->get_world_rotation();
					else
						return rotation * parent->get_world_rotation();
				}
				else
					return rotation;
			}

			_NODISCARD glm::vec<_Is2D ? 2 : 3, _FloatTy> get_world_position() const noexcept 
			{
				if (auto parent = myParent.lock()) _UNLIKELY
					return parent->get_world_position() + position;
				else
					return position;
			}

			_NODISCARD glm::vec<_Is2D ? 2 : 3, _FloatTy> get_world_scale() const noexcept
			{
				if (auto parent = myParent.lock()) _UNLIKELY
					return parent->get_world_scale() * scale;
				else
					return scale;
			}

			friend struct nlohmann::adl_serializer<BasicTransform>;

		private:
			size_t myPrevHash = -1;
			size_t myCurrentHash = 0;

			std::weak_ptr<BasicTransform const> myParent = {};

		public:
			rotator_type rotation = gen_default_rotation();

			glm::vec<_Is2D ? 2 : 3, _FloatTy> position { static_cast<_FloatTy>(0) };
			glm::vec<_Is2D ? 2 : 3, _FloatTy> scale    { static_cast<_FloatTy>(1) };
		};

		template <std::floating_point _FloatTy>
		using Transform = BasicTransform<_FloatTy, false>;

		template <std::floating_point _FloatTy>
		using Transform2D = BasicTransform<_FloatTy, true>;
	}
}

namespace nlohmann
{
	template <std::floating_point _FloatTy, bool _Use2D>
	struct adl_serializer <Coli::Geometry::BasicTransform<_FloatTy, _Use2D>>
	{
	private:
		static constexpr std::string_view name_position = "position";
		static constexpr std::string_view name_rotation = "rotation";
		static constexpr std::string_view name_scale    = "scale";

	public:
		static void to_json(json& j, Coli::Geometry::BasicTransform<_FloatTy, _Use2D> const& val)
		{
			j[name_position] = val.position;
			j[name_rotation] = val.rotation;
			j[name_scale]    = val.scale;
		}

		static void from_json(const json& j, Coli::Geometry::BasicTransform<_FloatTy, _Use2D>& val)
		{
			using Coli::Detail::Json::try_fill;

			decltype (val.position) tempPosition;
			try_fill (j, tempPosition, name_position);

			decltype (val.rotation) tempRotation;
			try_fill (j, tempRotation, name_rotation);

			decltype (val.scale) tempScale;
			try_fill (j, tempScale, name_scale);

			val.position = tempPosition;
			val.rotation = tempRotation;
			val.scale    = tempScale;

			val.myCurrentHash = 0;
			val.commit_change();
		}
	};
}
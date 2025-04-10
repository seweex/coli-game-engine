#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

namespace Coli
{
	namespace Geometry
	{
		template <std::floating_point _FloatTy, bool _Use2D>
		struct BasicVertex 
		{
			using float_type = _FloatTy;

			_NODISCARD constexpr bool operator==(BasicVertex const&) const noexcept = default;

			glm::vec<_Use2D ? 2 : 3, _FloatTy> position;
			glm::vec<2, _FloatTy>			   texcoord;
		};

		template <std::floating_point _FloatTy>
		using Vertex = BasicVertex<_FloatTy, false>;

		template <std::floating_point _FloatTy>
		using Vertex2D = BasicVertex<_FloatTy, true>;
	}

	namespace Detail
	{
		template <class _Ty>
		concept Vertex = requires (_Ty _val)
		{
			_val.position;
			_val.texcoord;

			typename _Ty::float_type;
		};

		template <Vertex _VertexTy>
		class VertexTraits final
		{
		public:
			using vertex_type = _VertexTy;
			using float_type  = typename _VertexTy::float_type;

			_NODISCARD static constexpr bool is_2D() noexcept {
				return position_length() == 2;
			}

			_NODISCARD static constexpr size_t position_length() noexcept {
				return sizeof(std::declval<_VertexTy>().position) / sizeof(typename _VertexTy::float_type);
			}

			_NODISCARD static constexpr size_t texcoord_length() noexcept {
				return sizeof(std::declval<_VertexTy>().texcoord) / sizeof(typename _VertexTy::float_type);
			}

			_NODISCARD static constexpr size_t position_offset() noexcept {
				return offsetof(_VertexTy, position);
			}

			_NODISCARD static constexpr size_t texcoord_offset() noexcept {
				return offsetof(_VertexTy, texcoord);
			}

			_NODISCARD static constexpr size_t vertex_size() noexcept {
				return sizeof(_VertexTy);
			}

			_NODISCARD static constexpr GLenum float_type_enum() noexcept 
			{
				if constexpr (std::same_as<float_type, float>)
					return GL_FLOAT;

				else if constexpr (std::same_as<float_type, double>)
					return GL_DOUBLE;

				else static_assert(false, "OpenGL doesn't support this float type");
			}
		};
	}
}

namespace std
{
	template <class _FloatTy, bool _Use2D>
	struct hash <Coli::Geometry::BasicVertex<_FloatTy, _Use2D>>
	{
		_NODISCARD size_t operator()(Coli::Geometry::BasicVertex<_FloatTy, _Use2D> const& val) const noexcept
		{			
			size_t hash = Coli::Detail::FNV::mix_hash(std::hash<decltype(val.position)>{}(val.position));
				   hash = Coli::Detail::FNV::mix_hash(std::hash<decltype(val.texcoord)>{}(val.texcoord), hash);

			return hash;
		}
	};
}
#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

namespace Coli
{
	namespace Geometry
	{
		template <bool _Use2D>
		struct BasicVertex 
		{
			_NODISCARD constexpr bool operator==(BasicVertex const&) const noexcept = default;

			glm::vec<_Use2D ? 2 : 3, double> position;
			glm::dvec2			             texcoord;
		};

		using Vertex   = BasicVertex <false>;
		using Vertex2D = BasicVertex <true>;
	}

	namespace Detail
	{
		template <class _Ty>
		concept Vertex = requires (_Ty _val)
		{
			_val.position;
			_val.texcoord;
		};

		template <Vertex _VertexTy>
		class VertexTraits final
		{
		public:
			using vertex_type = _VertexTy;

			_NODISCARD static constexpr bool is_2D() noexcept {
				return position_length() == 2;
			}

			_NODISCARD static constexpr size_t position_length() noexcept {
				return sizeof(_VertexTy::position) / sizeof(double);
			}

			_NODISCARD static constexpr size_t texcoord_length() noexcept {
				return sizeof(_VertexTy::texcoord) / sizeof(double);
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

			_NODISCARD static constexpr GLenum float_type_enum() noexcept {
				return GL_DOUBLE;
			}
		};
	}
}

namespace std
{
	template <bool _Use2D>
	struct hash <Coli::Geometry::BasicVertex<_Use2D>>
	{
		_NODISCARD size_t operator()(Coli::Geometry::BasicVertex<_Use2D> const& val) const noexcept
		{			
			Coli::Detail::HashMixer mixer;
			size_t hash;

			hash = mixer(std::hash<decltype(val.position)>{}(val.position));
			hash = mixer(std::hash<decltype(val.texcoord)>{}(val.texcoord), hash);

			return hash;
		}
	};
}
#pragma once

#include "../Common.hxx"

#include "Vertex.hxx"
#include "GlmHelper.hxx"

namespace Coli
{
	namespace Geometry
	{
		template <Detail::Vertex _VertexTy>
		class Mesh final
		{
			using vector_type = glm::vec <Detail::VertexTraits<_VertexTy>::position_length(), double>;
			
		public:
			template <Detail::IterableContainerOf <_VertexTy> _ContainerTy>
			Mesh (_ContainerTy const& vertices)
			{
				std::unordered_map <_VertexTy, unsigned> 
				uniques;
				
				auto const size = vertices.size();

				uniques.reserve(size);
				myVertices.reserve(size);
				myIndices.reserve(size);

				for (auto const& vertex : vertices)
				{
					auto [iter, inserted] = uniques.try_emplace(vertex, uniques.size());

					if (inserted)
						myVertices.push_back(vertex);

					myIndices.push_back(iter->second);
				}

				uniques.clear();
				myVertices.shrink_to_fit();
				myIndices.shrink_to_fit();
			}

			template <class _VerticesTy, class _IndicesTy>
			requires (
				Detail::IterableContainerOf <std::remove_reference_t <_VerticesTy>, _VertexTy> &&
				Detail::IterableContainerOf <std::remove_reference_t <_IndicesTy>,  unsigned>
			)
			Mesh (_VerticesTy&& vertices, _IndicesTy&& indices)
			{
				if constexpr (std::same_as <decltype(myVertices), std::remove_reference <_VerticesTy>>)
					myVertices = std::forward<_VerticesTy>(vertices);
				else
					myVertices.assign(vertices.begin(), vertices.end());
				
				if constexpr (std::same_as <decltype(myIndices), std::remove_reference <_IndicesTy>>)
					myIndices = std::forward<_IndicesTy>(indices);
				else
					myIndices.assign(indices.begin(), indices.end());
			}

			_NODISCARD std::span <_VertexTy const> get_vertices() const noexcept {
				return { myVertices.data(), myVertices.size() };
			}

			_NODISCARD std::span <unsigned const> get_indices() const noexcept {
				return { myIndices.data(), myIndices.size() };
			}

			_NODISCARD size_t size() const noexcept {
				return myIndices.size();
			}

		private:
			std::vector <_VertexTy> myVertices;
			std::vector <unsigned>  myIndices;
		};

		template <Detail::Vertex _VertexTy>
		class BoxMeshGenerator final
		{
			using position_type = glm::vec <Detail::VertexTraits<_VertexTy>::position_length(), double>;
			using texcoord_type = glm::vec <Detail::VertexTraits<_VertexTy>::texcoord_length(), double>;

			using vector_type = position_type;

			_NODISCARD static constexpr std::vector <_VertexTy> gen_vertices(vector_type const& size)
			{
				auto const halfSize = size / 2.0;

				if constexpr (Detail::VertexTraits<_VertexTy>::is_2D())
					return {
						{ { -halfSize.x, -halfSize.y }, { 0.0, 0.0 } },
						{ {  halfSize.x, -halfSize.y }, { 1.0, 0.0 } },
						{ {  halfSize.x,  halfSize.y }, { 1.0, 1.0 } },
						{ { -halfSize.x,  halfSize.y }, { 0.0, 1.0 } }
					};
				else 
					return {
						{ { -halfSize.x, -halfSize.y,  halfSize.z }, { 1.0 / 4.0, 1.0 / 3.0 } },
						{ {  halfSize.x, -halfSize.y,  halfSize.z }, { 2.0 / 4.0, 1.0 / 3.0 } },
						{ {  halfSize.x,  halfSize.y,  halfSize.z }, { 2.0 / 4.0, 2.0 / 3.0 } },
						{ { -halfSize.x,  halfSize.y,  halfSize.z }, { 1.0 / 4.0, 2.0 / 3.0 } },

						{ { -halfSize.x, -halfSize.y, -halfSize.z }, {3.0 / 4.0, 1.0 / 3.0 } },
						{ {  halfSize.x, -halfSize.y, -halfSize.z }, {4.0 / 4.0, 1.0 / 3.0 } },
						{ {  halfSize.x,  halfSize.y, -halfSize.z }, {4.0 / 4.0, 2.0 / 3.0 } },
						{ { -halfSize.x,  halfSize.y, -halfSize.z }, {3.0 / 4.0, 2.0 / 3.0 } },

						{ {  halfSize.x, -halfSize.y,  halfSize.z }, { 2.0 / 4.0, 1.0 / 3.0 } },
						{ {  halfSize.x, -halfSize.y, -halfSize.z }, { 3.0 / 4.0, 1.0 / 3.0 } },
						{ {  halfSize.x,  halfSize.y, -halfSize.z }, { 3.0 / 4.0, 2.0 / 3.0 } },
						{ {  halfSize.x,  halfSize.y,  halfSize.z }, { 2.0 / 4.0, 2.0 / 3.0 } },

						{ { -halfSize.x, -halfSize.y,  halfSize.z }, { 0.0 / 4.0, 1.0 / 3.0 } },
						{ { -halfSize.x, -halfSize.y, -halfSize.z }, { 1.0 / 4.0, 1.0 / 3.0 } },
						{ { -halfSize.x,  halfSize.y, -halfSize.z }, { 1.0 / 4.0, 2.0 / 3.0 } },
						{ { -halfSize.x,  halfSize.y,  halfSize.z }, { 0.0 / 4.0, 2.0 / 3.0 } },

						{ { -halfSize.x,  halfSize.y,  halfSize.z }, { 1.0 / 4.0, 2.0 / 3.0 } },
						{ {  halfSize.x,  halfSize.y,  halfSize.z }, { 2.0 / 4.0, 2.0 / 3.0 } },
						{ {  halfSize.x,  halfSize.y, -halfSize.z }, { 2.0 / 4.0, 3.0 / 3.0 } },
						{ { -halfSize.x,  halfSize.y, -halfSize.z }, { 1.0 / 4.0, 3.0 / 3.0 } },

						{ { -halfSize.x, -halfSize.y,  halfSize.z }, { 1.0 / 4.0, 1.0 / 3.0 } },
						{ {  halfSize.x, -halfSize.y,  halfSize.z }, { 2.0 / 4.0, 1.0 / 3.0 } },
						{ {  halfSize.x, -halfSize.y, -halfSize.z }, { 2.0 / 4.0, 0.0 / 3.0 } },
						{ { -halfSize.x, -halfSize.y, -halfSize.z }, { 1.0 / 4.0, 0.0 / 3.0 } }
					};
			}

			_NODISCARD static constexpr std::vector <unsigned> gen_indices()
			{
				if constexpr (Detail::VertexTraits<_VertexTy>::is_2D())
					return {
						0, 1, 2,
						0, 2, 3
					};
				else
					return {
						0, 3, 2,		2, 1, 0,
						4, 6, 7,		6, 4, 5,
						8, 11, 10,      10, 9, 8,
						15, 12, 14,		13, 14, 12,
						16, 19, 18,		18, 17, 16,
						20, 22, 23,		22, 20, 21,
					};
			}

		public:
			_NODISCARD constexpr Mesh <_VertexTy> operator()(vector_type const& sizes) const
			{
				static const auto indices  = gen_indices();
				auto const        vertices = gen_vertices(sizes);

				return Mesh <_VertexTy>(std::move(vertices), indices);
			}
		};

		template <Detail::Vertex _VertexTy>
		class SphereMeshGenerator final
		{
			using position_type = glm::vec <Detail::VertexTraits<_VertexTy>::position_length(), double>;
			using texcoord_type = glm::vec <Detail::VertexTraits<_VertexTy>::texcoord_length(), double>;

			using vector_type = position_type;

			static void verify_density (unsigned density) 
			{
				auto constexpr min_density = 3;
				auto constexpr max_density = 100'000;

				if (density > max_density || density < min_density)
					throw std::invalid_argument("Invalid density value");
			}

			_NODISCARD static std::vector <_VertexTy> gen_vertices (
				double radius,
				unsigned meridianDensity, 
				unsigned parallelDensity
			) {
				std::vector <_VertexTy> vertices;
				vertices.reserve (parallelDensity * meridianDensity);

				for (unsigned i = 0; i <= parallelDensity; ++i)
				{
					auto const theta    = i * glm::pi<double>() / parallelDensity;
					auto const sinTheta = sin(theta);
					auto const cosTheta = cos(theta);

					for (unsigned j = 0; j <= meridianDensity; ++j)
					{
						auto const phi    = j * 2.0 * glm::pi<double>() / meridianDensity;
						auto const sinPhi = sin(phi);
						auto const cosPhi = cos(phi);

						auto const x = radius * sinTheta * cosPhi;
						auto const y = radius * sinTheta * sinPhi;
						auto const z = radius * cosTheta;

						auto const u = static_cast<double>(j) / meridianDensity;
						auto const v = static_cast<double>(i) / parallelDensity;

						vertices.emplace_back(position_type{ x, y, z }, texcoord_type{ u, v });
					}
				}

				return vertices;
			}

			_NODISCARD static std::vector <unsigned> gen_indices (
				unsigned meridianDensity,
				unsigned parallelDensity
			) {				 
				std::vector <unsigned> indices;
				indices.reserve (6ull * meridianDensity * parallelDensity);

				for (unsigned i = 0; i < parallelDensity; ++i)
				for (unsigned j = 0; j < meridianDensity; ++j)
				{
					auto const first  = (i * (meridianDensity + 1)) + j;
					auto const second = first + meridianDensity + 1;

					indices.push_back (first);
					indices.push_back (first + 1);
					indices.push_back (second);

					indices.push_back (second);
					indices.push_back (first + 1);
					indices.push_back (second + 1);
				}

				return indices;
			}

		public:
			_NODISCARD Mesh <_VertexTy> operator()(double radius, unsigned meridians, unsigned parallels) const
			{
				verify_density (meridians);
				verify_density (parallels);

				auto const indices  = gen_indices  (meridians, parallels);
				auto const vertices = gen_vertices (radius, meridians, parallels);

				return Mesh <_VertexTy> { vertices, indices };
			}
		};
	}
}
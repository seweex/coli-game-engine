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
			using float_type    = typename Detail::VertexTraits<_VertexTy>::float_type;
			using vector_type   = glm::vec <Detail::VertexTraits<_VertexTy>::position_length(), float_type>;
			using texcoord_type = glm::vec <Detail::VertexTraits<_VertexTy>::texcoord_length(), float_type>;

			_NODISCARD static constexpr std::vector<_VertexTy> gen_box_vertices(vector_type const& size)
			{
				auto const halfSize = size / static_cast<typename Detail::VertexTraits<_VertexTy>::float_type>(2);

				if constexpr (Detail::VertexTraits<_VertexTy>::is_2D())
					return {
						{ {-halfSize.x, -halfSize.y}, {0.0, 0.0} },
						{ { halfSize.x, -halfSize.y}, {1.0, 0.0} },
						{ { halfSize.x,  halfSize.y}, {1.0, 1.0} },
						{ {-halfSize.x,  halfSize.y}, {0.0, 1.0} }
					};
				else 
					return {
						{ {-halfSize.x, -halfSize.y,  halfSize.z}, {1.0 / 4.0, 1.0 / 3.0} },
						{ { halfSize.x, -halfSize.y,  halfSize.z}, {2.0 / 4.0, 1.0 / 3.0} },
						{ { halfSize.x,  halfSize.y,  halfSize.z}, {2.0 / 4.0, 2.0 / 3.0} },
						{ {-halfSize.x,  halfSize.y,  halfSize.z}, {1.0 / 4.0, 2.0 / 3.0} },

						{ {-halfSize.x, -halfSize.y, -halfSize.z}, {3.0 / 4.0, 1.0 / 3.0} },
						{ { halfSize.x, -halfSize.y, -halfSize.z}, {4.0 / 4.0, 1.0 / 3.0} },
						{ { halfSize.x,  halfSize.y, -halfSize.z}, {4.0 / 4.0, 2.0 / 3.0} },
						{ {-halfSize.x,  halfSize.y, -halfSize.z}, {3.0 / 4.0, 2.0 / 3.0} },

						{ { halfSize.x, -halfSize.y,  halfSize.z}, {2.0 / 4.0, 1.0 / 3.0} },
						{ { halfSize.x, -halfSize.y, -halfSize.z}, {3.0 / 4.0, 1.0 / 3.0} },
						{ { halfSize.x,  halfSize.y, -halfSize.z}, {3.0 / 4.0, 2.0 / 3.0} },
						{ { halfSize.x,  halfSize.y,  halfSize.z}, {2.0 / 4.0, 2.0 / 3.0} },

						{ {-halfSize.x, -halfSize.y,  halfSize.z}, {0.0 / 4.0, 1.0 / 3.0} },
						{ {-halfSize.x, -halfSize.y, -halfSize.z}, {1.0 / 4.0, 1.0 / 3.0} },
						{ {-halfSize.x,  halfSize.y, -halfSize.z}, {1.0 / 4.0, 2.0 / 3.0} },
						{ {-halfSize.x,  halfSize.y,  halfSize.z}, {0.0 / 4.0, 2.0 / 3.0} },

						{ {-halfSize.x,  halfSize.y,  halfSize.z}, {1.0 / 4.0, 2.0 / 3.0} },
						{ { halfSize.x,  halfSize.y,  halfSize.z}, {2.0 / 4.0, 2.0 / 3.0} },
						{ { halfSize.x,  halfSize.y, -halfSize.z}, {2.0 / 4.0, 3.0 / 3.0} },
						{ {-halfSize.x,  halfSize.y, -halfSize.z}, {1.0 / 4.0, 3.0 / 3.0} },

						{ {-halfSize.x, -halfSize.y,  halfSize.z}, {1.0 / 4.0, 1.0 / 3.0} },
						{ { halfSize.x, -halfSize.y,  halfSize.z}, {2.0 / 4.0, 1.0 / 3.0} },
						{ { halfSize.x, -halfSize.y, -halfSize.z}, {2.0 / 4.0, 0.0 / 3.0} },
						{ {-halfSize.x, -halfSize.y, -halfSize.z}, {1.0 / 4.0, 0.0 / 3.0} }
					};
			}

			_NODISCARD static constexpr std::vector<unsigned> gen_box_indices()
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

			_NODISCARD static constexpr std::vector<_VertexTy> gen_circle_vertices(
				float_type radius,
				unsigned meridianDensity, 
				unsigned parallelDensity
			) {
				std::vector<_VertexTy> vertices;
				vertices.reserve(parallelDensity * meridianDensity);

				for (size_t i = 0; i <= parallelDensity; ++i) 
				{
					float_type const theta    = i * glm::pi<float_type>() / parallelDensity;
					float_type const sinTheta = sin(theta);
					float_type const cosTheta = cos(theta);

					for (size_t j = 0; j <= meridianDensity; ++j) 
					{
						float_type const phi    = j * 2 * glm::pi<float_type>() / meridianDensity;
						float_type const sinPhi = sin(phi);
						float_type const cosPhi = cos(phi);

						float_type const x = radius * sinTheta * cosPhi;
						float_type const y = radius * sinTheta * sinPhi;
						float_type const z = radius * cosTheta;

						float_type const u = static_cast<float_type>(j) / meridianDensity;
						float_type const v = static_cast<float_type>(i) / parallelDensity;

						vertices.emplace_back(vector_type(x, y, z), texcoord_type(u, v));
					}
				}

				return vertices;
			}

			_NODISCARD static constexpr std::vector<unsigned> gen_circle_indices(
				unsigned meridianDensity,
				unsigned parallelDensity
			) {
				std::vector<unsigned int> indices;
				indices.reserve(meridianDensity * parallelDensity * 6);

				for (size_t i = 0; i < parallelDensity; ++i)
				for (size_t j = 0; j < meridianDensity; ++j)
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
			template <Detail::LinearContainerOf<_VertexTy> _ContainerTy>
			Mesh (_ContainerTy const& vertices)
			{
				std::unordered_map <_VertexTy, unsigned> 
				uniques;
				
				uniques.    reserve(vertices.size());
				myVertices. reserve(vertices.size());
				myIndices.  reserve(vertices.size());

				for (auto const& vertex : vertices)
				{
					auto [iter, inserted] = uniques.try_emplace(vertex, uniques.size());

					if (inserted) _LIKELY
						myVertices.push_back(vertex);

					myIndices.push_back(iter->second);
				}
			}

			Mesh (vector_type const& size) :
				myVertices (gen_box_vertices(size)),
				myIndices  (gen_box_indices())
			{}

			Mesh (
				float_type radius,
				unsigned   meridianDensity,
				unsigned   parallelDensity
			) :
				myVertices (gen_circle_vertices (radius, meridianDensity, parallelDensity)),
				myIndices  (gen_circle_indices  (meridianDensity, parallelDensity))
			{}

			_NODISCARD std::span<_VertexTy const> get_vertices() const noexcept {
				return { myVertices.data(), myVertices.size() };
			}

			_NODISCARD std::span<unsigned const> get_indices() const noexcept {
				return { myIndices.data(), myIndices.size() };
			}

			_NODISCARD size_t get_vertices_count() const noexcept {
				return myIndices.size();
			}

		private:
			std::vector<_VertexTy> myVertices;
			std::vector<unsigned>  myIndices;
		};
	}
}
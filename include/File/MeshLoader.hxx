#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Loader.hxx"

#include "../Geometry/Mesh.hxx"

namespace Coli
{
	namespace File
	{
		class MeshLoader final :
			public Detail::FileLoaderBase
		{
			static void x_parse_error (std::string_view description) 
			{
				using namespace std::literals;

				throw std::runtime_error("Failed to parse the file: "s + description.data());
			}

		public:
			MeshLoader(std::string_view rootPath) :
				FileLoaderBase (rootPath, ".obj")
			{}

			MeshLoader(MeshLoader&&)      = delete;
			MeshLoader(MeshLoader const&) = delete;

			MeshLoader& operator=(MeshLoader&&)      = delete;
			MeshLoader& operator=(MeshLoader const&) = delete;

			template <Detail::Vertex _VertexTy>
			_NODISCARD Geometry::Mesh <_VertexTy> load (std::string_view assetName)
			{
				auto path = this->make_path(assetName);

				tinyobj::ObjReader reader;
				reader.ParseFromFile(path);
				
				auto const& error = reader.Error();

				if (!error.empty())
					x_parse_error (error);

				auto const& attrib = reader.GetAttrib();
				auto const& shapes = reader.GetShapes();

				std::vector <_VertexTy> vertices;
				vertices.reserve (attrib.vertices.size() / 3);

				for (auto const& shape : shapes)
				for (auto const& index : shape.mesh.indices)
				{
					auto& vertex = vertices.emplace_back();

					vertex.position.x = attrib.vertices [3 * index.vertex_index];
					vertex.position.y = attrib.vertices [3 * index.vertex_index + 1];

					if constexpr (!Detail::VertexTraits<_VertexTy>::is_2D())
						vertex.position.z = attrib.vertices [3 * index.vertex_index + 2];

					if (index.texcoord_index >= 0) {
						vertex.texcoord.x = attrib.texcoords [2 * index.texcoord_index];
						vertex.texcoord.y = attrib.texcoords [2 * index.texcoord_index + 1];
					}
					else
						vertex.texcoord.x = vertex.texcoord.y = 0;
				}

				return Geometry::Mesh<_VertexTy>{ vertices };
			}
		};
	}
}
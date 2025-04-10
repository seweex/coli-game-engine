#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Context.hxx"
#include "Buffer.hxx"
#include "VertexArray.hxx"
#include "Material.hxx"

#include "../Geometry/Mesh.hxx"
#include "../Geometry/Transform.hxx"

namespace Coli
{
	namespace Graphics
	{
		class Renderer;
	}

	namespace Detail
	{
		inline namespace OpenGL
		{
			struct ModelUniformBlock
			{
				static constexpr unsigned binding_index = 1;

				glm::mat4 model = { 1 };
			};

			class ModelContext
			{
			public:
				ModelContext() :
					myBuffer (default_value)
				{}

				template <class _FloatTy, bool _Is2D>
				void update(Geometry::BasicTransform<_FloatTy, _Is2D> const& transform) noexcept {
					if (transform.has_changed()) _LIKELY
						myBuffer.write(static_cast<glm::mat4>(transform.get_model_matrix()), offsetof(ModelUniformBlock, model));
				}

				void bind() {
					myBuffer.bind(ModelUniformBlock::binding_index);
				}

				void unbind() noexcept {
					Graphics::UniformBuffer::unbind(ModelUniformBlock::binding_index);
				}

			private:
				static constexpr ModelUniformBlock default_value = {};

				Graphics::UniformBuffer myBuffer;
			};

			template <Vertex _VertexTy>
			class MeshContext
			{
			public:
				MeshContext (Geometry::Mesh<_VertexTy> const& mesh) :
					myVerticesCount (mesh.get_vertices_count()),
					myVertices (mesh.get_vertices()),
					myIndices  (mesh.get_indices()),
					myVAO	   (myVertices)
				{}

				_NODISCARD size_t get_vertices_count() const noexcept {
					return myVerticesCount;
				}

				void bind() {
					myVAO.bind();
					myVertices.bind();
					myIndices.bind();
				}

				static void unbind() noexcept {
					Graphics::IndexStorage::unbind();
					Graphics::VertexStorage::unbind();
					Graphics::VertexArray<_VertexTy>::unbind();
				}

			private:
				size_t const myVerticesCount;

				Graphics::VertexStorage myVertices;
				Graphics::IndexStorage  myIndices;
				Graphics::VertexArray<_VertexTy> myVAO;
			};

			class DrawableBase 
			{
				_NODISCARD static std::shared_ptr<Graphics::Renderer> take_renderer()
				{
					if (current_renderer.expired()) {
						auto newRenderer = std::make_shared<Graphics::Renderer>();
						current_renderer = newRenderer;
						return newRenderer;
					}
					else
						return current_renderer.lock();
				}

			public:
				DrawableBase() :
					myRenderer (take_renderer())
				{}

				_NODISCARD static std::weak_ptr<Graphics::Renderer> get_renderer() noexcept {
					return current_renderer;
				}

			private:
				static inline std::weak_ptr<Graphics::Renderer> current_renderer;

			protected:
				std::shared_ptr<Graphics::Renderer> myRenderer;
			};
		}
	}

	namespace Graphics
	{
		template <Detail::Vertex _VertexTy>
		class Drawable :
			public Detail::MeshContext<_VertexTy>,
			public Detail::ModelContext,
			public Detail::DrawableBase
		{
			using _Mesh_base  = Detail::MeshContext<_VertexTy>;
			using _Model_base = Detail::ModelContext;

		public:
			Drawable (Geometry::Mesh<_VertexTy> const& mesh) :
				_Mesh_base (mesh)
			{}

			void set_material(std::shared_ptr<Detail::MaterialBase> material) {
				myMaterial.swap(material);
			}

			void bind() {
				_Mesh_base::bind();
				_Model_base::bind();
				myMaterial->bind();
			}

			void unbind() noexcept {
				myMaterial->unbind();
				_Model_base::unbind();
				_Mesh_base::unbind();
			}

		private:
			std::shared_ptr<Detail::MaterialBase> myMaterial;
		};
	}
}
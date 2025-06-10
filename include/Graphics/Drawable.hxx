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

				ModelContext(ModelContext&&)	  = delete;
				ModelContext(ModelContext const&) = delete;

				ModelContext& operator=(ModelContext&&)		 = delete;
				ModelContext& operator=(ModelContext const&) = delete;

				template <bool _Is2D>
				void update (Geometry::BasicTransform <_Is2D> const& transform) noexcept {
					if (transform.has_changed())
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
					myVerticesCount (mesh.size()),
					myVertices		(mesh.get_vertices()),
					myIndices		(mesh.get_indices()),
					myVAO			(myVertices)
				{}

				MeshContext(MeshContext&&)		= delete;
				MeshContext(MeshContext const&) = delete;

				MeshContext& operator=(MeshContext&&)	   = delete;
				MeshContext& operator=(MeshContext const&) = delete;

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
				size_t myVerticesCount;

				Graphics::VertexStorage myVertices;
				Graphics::IndexStorage  myIndices;
				Graphics::VertexArray<_VertexTy> myVAO;
			};

			class DrawableBase 
			{
			public:
				DrawableBase() noexcept = default;

				DrawableBase(DrawableBase&&)	  = delete;
				DrawableBase(DrawableBase const&) = delete;

				DrawableBase& operator=(DrawableBase&&)		 = delete;
				DrawableBase& operator=(DrawableBase const&) = delete;

				static void set_renderer (std::weak_ptr <Graphics::Renderer> renderer) noexcept {
					current_renderer = renderer;
				}

			private:
				static inline std::weak_ptr <Graphics::Renderer> current_renderer;

			protected:
				_NODISCARD std::shared_ptr <Graphics::Renderer> get_renderer() noexcept {
					return current_renderer.lock();
				}
			};
		}
	}

	namespace Graphics
	{
		template <Detail::Vertex _VertexTy>
		class Drawable :
			public Detail::MeshContext <_VertexTy>,
			public Detail::ModelContext,
			public Detail::DrawableBase
		{
			using mesh_base  = Detail::MeshContext<_VertexTy>;
			using model_base = Detail::ModelContext;

		public:
			Drawable (Geometry::Mesh<_VertexTy> const& mesh) :
				mesh_base (mesh)
			{}

			Drawable(Drawable&&)	  = delete;
			Drawable(Drawable const&) = delete;
			 
			Drawable& operator=(Drawable&&)		 = delete;
			Drawable& operator=(Drawable const&) = delete;

			void set_material (std::weak_ptr <Detail::MaterialBase> material) {
				myMaterial = material.lock();
			}

			void bind() {
				mesh_base::bind();
				model_base::bind();
				myMaterial->bind();
			}

			void unbind() noexcept {
				myMaterial->unbind();
				model_base::unbind();
				mesh_base::unbind();
			}

		private:
			std::shared_ptr <Detail::MaterialBase> myMaterial;
		};
	}
}
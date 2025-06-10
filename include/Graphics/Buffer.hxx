#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Context.hxx"

namespace Coli
{
	namespace Graphics
	{
		enum class BufferType {
			index,
			vertex,
			uniform
		};
	}

	namespace Detail
	{
		template <Graphics::BufferType _Type> 
		requires (
			_Type == Graphics::BufferType::vertex  ||
			_Type == Graphics::BufferType::uniform ||
			_Type == Graphics::BufferType::index   
		)
		class BufferBase :
			public Detail::ContextDependBase
		{
		protected:
			_NODISCARD static constexpr GLenum get_type_gl() noexcept
			{
				switch (_Type)
				{
				case Graphics::BufferType::index:
					return GL_ELEMENT_ARRAY_BUFFER;

				case Graphics::BufferType::vertex:
					return GL_ARRAY_BUFFER;

				case Graphics::BufferType::uniform:
					return GL_UNIFORM_BUFFER;
				}
			}

			template <class _Ty> requires (std::is_object_v<_Ty>)
			_NODISCARD static DataProxy take_memory (_Ty const& val) noexcept
			{
				if constexpr (std::is_array_v<_Ty>)
					return { val, sizeof(val) };

				else if constexpr (std::indirectly_readable <_Ty>)
					return { &*val, sizeof(*val) };

				else if constexpr (LinearContainer <_Ty>)
					return { val.data(), val.size() * sizeof(typename _Ty::value_type) };

				else return { &val, sizeof(val) };
			}

		private:
			static void x_failed_create() {
				throw std::runtime_error("Failed to create a buffer");
			}

			static void x_invalid_index() {
				throw std::invalid_argument("Invalid index");
			}

		public:
			BufferBase()
			{
				glGenBuffers (1, &myHandle);
				
				if (myHandle == 0)
					x_failed_create();

				this->bind();
				this->unbind();
			}

			~BufferBase() noexcept {
				glDeleteBuffers(1, &myHandle);
			}

			BufferBase(BufferBase&&)	  = delete;
			BufferBase(BufferBase const&) = delete;

			BufferBase& operator=(BufferBase&&)		 = delete;
			BufferBase& operator=(BufferBase const&) = delete;

			void bind() noexcept {
				if (myHandle != current_binding)
					glBindBuffer(get_type_gl(), current_binding = myHandle);
			}

			static void unbind() noexcept {
				glBindBuffer(get_type_gl(), current_binding = 0);
			}

			void bind (unsigned index) {
				if (index < indexed_bindings.size()) {
					if (myHandle != indexed_bindings[index])
						glBindBufferBase (get_type_gl(), index, indexed_bindings[index] = myHandle);
				}
				else
					x_invalid_index();
			}

			static void unbind (unsigned index) noexcept {
				if (index < indexed_bindings.size())
					glBindBufferBase(get_type_gl(), index, indexed_bindings[index] = 0);
			}

		private:
			static inline std::array<GLuint, 5> indexed_bindings = { 0 };
			static inline GLuint				current_binding  = 0;

		protected:
			GLuint myHandle = 0;
		};
	}

	namespace Graphics
	{
		template <BufferType _Type>
		class Buffer final :
			public Detail::BufferBase<_Type>
		{
		public:
			Buffer (auto const& initData) :
				mySize (0),
				myData (nullptr)
			{
				assign (initData);
			}

			~Buffer() noexcept {
				glUnmapNamedBuffer(myHandle);
			}

			void assign(auto const& initData) noexcept 
			{
				auto [data, size] = Detail::BufferBase<_Type>::take_memory(initData);

				if (size <= mySize)
				{
					if (size < mySize)
						memset(myData + size, 0, mySize - size);

					memcpy_s(myData, mySize, data, size);
					glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
				}
				else {
					if (myData)
						glUnmapNamedBuffer (myHandle);
					
					glNamedBufferStorage (myHandle, size, data, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
					
					mySize = size;
					myData = reinterpret_cast<uint8_t*>(glMapNamedBuffer(myHandle, GL_WRITE_ONLY));
				}
			}

			void write(auto const& initData, size_t offset)
			{
				auto [data, size] = Detail::BufferBase<_Type>::take_memory(initData);

				if (size + offset <= mySize) {
					memcpy_s(myData + offset, mySize - offset, data, size);
					glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
				}
				else
					throw std::overflow_error("the buffer overflowed");
			}

		private:
			using Detail::BufferBase<_Type>::myHandle;

			size_t   mySize;
			uint8_t* myData;
		};

		template <BufferType _Type>
		class Storage final :
			public Detail::BufferBase<_Type>
		{
		public:
			Storage (auto const& initData)
			{
				auto [data, size] = Detail::BufferBase<_Type>::take_memory(initData);
			
				glNamedBufferStorage(myHandle, size, data, 0);
			}

		private:
			using Detail::BufferBase<_Type>::myHandle;
		};

		using VertexStorage  = Storage <BufferType::vertex>;
		using IndexStorage   = Storage <BufferType::index>;
		using UniformStorage = Storage <BufferType::uniform>;

		using VertexBuffer  = Buffer <BufferType::vertex>;
		using IndexBuffer   = Buffer <BufferType::index>;
		using UniformBuffer = Buffer <BufferType::uniform>;
	}
}
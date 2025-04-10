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
		class BufferBase
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

		protected:
			_NODISCARD static std::pair<const void*, size_t> take_memory(auto const& val) noexcept 
			{
				using unspecified_type = std::remove_cvref_t<decltype(val)>;

				if constexpr (std::is_array_v<unspecified_type>)
					return { val, sizeof(val) };

				else if constexpr (std::indirectly_readable<unspecified_type>)
					return { &*val, sizeof(*val) };

				else if constexpr (LinearContainer<unspecified_type>)
					return { val.data(), val.size() * sizeof(typename unspecified_type::value_type) };

				else return { &val, sizeof(val) };
			}

		public:
			BufferBase()
			{
				if (!Graphics::Context::is_ready())
					throw std::runtime_error("no ready context");
				
				glGenBuffers(1, &myHandle);
				
				if (myHandle == 0)
					throw std::runtime_error("failed to create a buffer");

				this->bind();
				this->unbind();
			}

			~BufferBase() noexcept {
				glDeleteBuffers(1, &myHandle);
			}

			void bind() noexcept {
				if (myHandle != current_binding)
					glBindBuffer(get_type_gl(), current_binding = myHandle);
			}

			static void unbind() noexcept {
				glBindBuffer(get_type_gl(), current_binding = 0);
			}

			void bind(unsigned index) {
				if (index < indexed_bindings.size()) _LIKELY {
					if (myHandle != indexed_bindings[index]) _LIKELY
						glBindBufferBase(get_type_gl(), index, indexed_bindings[index] = myHandle);
				} 
				else
					throw std::invalid_argument("invalid index");				
			}

			static void unbind(unsigned index) noexcept {
				if (index < indexed_bindings.size()) _LIKELY
					glBindBufferBase(get_type_gl(), index, indexed_bindings[index] = 0);
			}

		private:
			static inline std::array<GLuint, 5> indexed_bindings = { 0 };
			static inline GLuint				current_binding  = 0;

		protected:
			GLuint myHandle = 0;
		};

		using VertexBufferBase = BufferBase<Graphics::BufferType::vertex>;
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
				assign(initData);
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

		using VertexStorage = Storage <BufferType::vertex>;
		using IndexStorage  = Storage <BufferType::index>;
		using UniformBuffer = Buffer  <BufferType::uniform>;
	}
}
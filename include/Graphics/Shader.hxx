#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

namespace Coli
{
	namespace Graphics
	{
		enum class ShaderType {
			vertex,
			fragment,
			geometry
		};

		template <ShaderType _Type>
		requires (
			_Type == ShaderType::vertex   ||
			_Type == ShaderType::geometry ||
			_Type == ShaderType::fragment 
		)
		class Shader final :
			public Detail::ContextDependBase
		{
			_NODISCARD static constexpr GLenum get_type_gl() noexcept 
			{
				switch (_Type)
				{
				case ShaderType::vertex:
					return GL_VERTEX_SHADER;

				case ShaderType::fragment:
					return GL_FRAGMENT_SHADER;

				case ShaderType::geometry:
					return GL_GEOMETRY_SHADER;
				}
			}

			static void x_failed_create() {
				throw std::runtime_error("Failed to create a shader");
			}

			static void x_failed_compile() {
				throw std::runtime_error("Failed to compile the shader");
			}

		public:
			Shader (std::string_view sourceCode) 
			{
				if ((myHandle = glCreateShader(get_type_gl())) == 0)
					x_failed_create();

				auto data = sourceCode.data();
				auto size = static_cast<int>(sourceCode.size());

				glShaderSource  (myHandle, 1, &data, &size);
				glCompileShader (myHandle);

				GLint flag;
				glGetShaderiv(myHandle, GL_COMPILE_STATUS, &flag);

				if (flag == GL_FALSE)
					x_failed_compile();
			}

			~Shader() noexcept {
				glDeleteShader(myHandle);
			}

			Shader(Shader&&)	  = delete;
			Shader(Shader const&) = delete;

			Shader& operator=(Shader&&)		 = delete;
			Shader& operator=(Shader const&) = delete;

			friend class Program;

		private:
			GLuint myHandle = 0;
		};

		using VertexShader   = Shader <ShaderType::vertex>;
		using FragmentShader = Shader <ShaderType::fragment>;
		using GeometryShader = Shader <ShaderType::geometry>;

		namespace ShaderCode 
		{
			inline constexpr std::string_view vertex = {
				"#version 450 core\n"

				"layout (location = 0) in vec3 inPosition;\n"
				"layout (location = 1) in vec2 inTexcoord;\n"

				"layout (std140, binding = 0) uniform CameraBlock\n"
				"{\n"
					"mat4 cameraViewMat;\n"
					"mat4 cameraProjMat;\n"
				"};\n"

				"layout (std140, binding = 1) uniform ModelBlock\n"
				"{\n"
					"mat4 modelMat;\n"
				"};\n"

				"out vec2 imTexcoord;\n"

				"void main()\n"
				"{\n"
					"imTexcoord  = inTexcoord;\n"
					"gl_Position = cameraProjMat * cameraViewMat * modelMat * vec4(inPosition, 1);\n"
				"}\n"
			};

			inline constexpr std::string_view pixel = {
				"#version 450 core\n"

				"in vec2 imTexcoord;\n"

				"layout (binding = 0) uniform sampler2D diffuseTexture;\n"

				"out vec4 color;\n"

				"void main()\n"
				"{\n"
					"color = texture(diffuseTexture, imTexcoord);\n"
				"}\n"
			};
		}
	}
}
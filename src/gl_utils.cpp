#include "gl_utils.h"

#include <glm/glm.hpp>
#include <stb_image.h>


const char *gl_get_error_string(GLenum error)
{
	switch (error)
	{
	case GL_NO_ERROR:          return "No Error";
	case GL_INVALID_ENUM:      return "Invalid Enum";
	case GL_INVALID_VALUE:     return "Invalid Value";
	case GL_INVALID_OPERATION: return "Invalid Operation";
	case GL_INVALID_FRAMEBUFFER_OPERATION: return "Invalid Framebuffer Operation";
	case GL_OUT_OF_MEMORY:     return "Out of Memory";
	case GL_STACK_UNDERFLOW:   return "Stack Underflow";
	case GL_STACK_OVERFLOW:    return "Stack Overflow";
	case GL_CONTEXT_LOST:      return "Context Lost";
	default:                   return "Unknown Error";
	}
}

struct Vertex {
	glm::vec3 pos;
	glm::vec2 uv;
};

namespace gl_utils {

	void gl_debug_msg(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
		const GLchar *message, const void *userParam)
	{
		if (source == GL_DEBUG_SOURCE_SHADER_COMPILER) return;

		if (severity == GL_DEBUG_SEVERITY_LOW) {
			CORE_INFO("OpenGL Info: {}", gl_get_error_string(type));
			CORE_INFO("{}", message);
		}
		else if (severity == GL_DEBUG_SEVERITY_MEDIUM) {
			CORE_WARN("OpenGL Error: {}", gl_get_error_string(type));
			CORE_WARN("{}", message);
		}
		else if (severity == GL_DEBUG_SEVERITY_HIGH) {
			CORE_ERROR("OpenGL Error: {}", gl_get_error_string(type));
			CORE_ERROR("{}", message);
		}
	}

	bool load_shader_module(const char *filePath, GLenum shaderType, uint32_t *shaderID) {

		uint32_t id = glCreateShader(shaderType);

		std::string shaderCode;
		std::ifstream file(filePath, std::ios::in);

		if (!file.is_open()) {
			return false;
		}

		std::stringstream sstr;
		sstr << file.rdbuf();
		shaderCode = sstr.str();
		file.close();

		char const *sourcePtr = shaderCode.c_str();
		glShaderSource(id, 1, &sourcePtr, nullptr);
		glCompileShader(id);


		int32_t res = GL_FALSE;
		int infoLogLength;

		glGetShaderiv(id, GL_COMPILE_STATUS, &res);
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (infoLogLength > 0) {
			std::string msg;
			msg.resize(size_t(infoLogLength + 1));

			//TODO: error handling
			glGetShaderInfoLog(id, infoLogLength, nullptr, msg.data());
			CORE_WARN("Shader Compilation: {}", filePath);
			CORE_WARN("{}", msg);
			return false;
		}

		*shaderID = id;
		return true;
	}

	bool link_shader_modules(uint32_t *modules, uint32_t moduleCount, uint32_t *programID) {

		uint32_t id = glCreateProgram();

		for (uint32_t i = 0; i < moduleCount; i++) {
			glAttachShader(id, modules[i]);
		}

		glLinkProgram(id);

		{
			int32_t res = GL_FALSE;
			int infoLogLength{};
			glGetProgramiv(id, GL_LINK_STATUS, &res);
			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
			if (infoLogLength > 0) {
				std::string msg;
				msg.resize(size_t(infoLogLength + 1));

				//TODO: error handling
				glGetProgramInfoLog(id, infoLogLength, nullptr, msg.data());
				CORE_WARN("Shader Linking:");
				CORE_WARN("{}", msg);
				return false;
			}
		}

		for (uint32_t i = 0; i < moduleCount; i++) glDetachShader(id, modules[i]);

		*programID = id;
		return true;
	}

	bool load_shader(const char *vertexFilePath, const char *fragmentFilePath, uint32_t *shader) {
		bool res = false;

		uint32_t modules[2]{};
		res = load_shader_module("assets/shaders/default.vert", GL_VERTEX_SHADER, &modules[0]);
		if (!res) return false;

		uint32_t fragShaderID{};
		res = load_shader_module("assets/shaders/default.frag", GL_FRAGMENT_SHADER, &modules[1]);
		if (!res) return false;

		uint32_t id{};
		res = link_shader_modules(modules, 2, &id);
		if (!res) return false;

		glDeleteShader(modules[0]);
		glDeleteShader(modules[1]);

		*shader = id;
		return true;
	}

	struct GLTexture {
		uint32_t width, height, nChannels;
		uint32_t id;
	};

	bool load_texture(const char *filePath, GLTexture *texture) {

		int width, height, channels;

		stbi_uc *data = nullptr;
		data = stbi_load(filePath, &width, &height, &channels, 0);

		if (!data || stbi_failure_reason()) {
			CORE_WARN("Failed to load image: {}", filePath);
			CORE_WARN("{}", stbi_failure_reason());
			return false;
		}

		GLenum internalFormat = 0;
		GLenum dataFormat = 0;

		if (channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}

		uint32_t id;
		glCreateTextures(GL_TEXTURE_2D, 1, &id);

		glTextureStorage2D(id, 1, internalFormat, width, height);
		glTextureSubImage2D(id, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateTextureMipmap(id);

		glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);

		texture->id = id;
		texture->width = width;
		texture->height = height;

		return true;
	}

	uint32_t vertexArrayID;
	uint32_t VBO, VAO, EBO;
	uint32_t shader;
	GLTexture texture;

	void init()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
		glDebugMessageCallback(gl_debug_msg, 0);

		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

		Vertex vertices[] =
		{
			{{0, 0, 0}, {0, 0}},
			{{0, 1, 0}, {0, 1}},
			{{1, 1, 0}, {1, 1}},
			{{1, 0, 0}, {1, 0}},
		};

		uint32_t indices[] =
		{
			0, 1, 2,
			0, 2, 3
		};

		glCreateVertexArrays(1, &VAO);
		glCreateBuffers(1, &VBO);
		glCreateBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glNamedBufferData(VBO, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glNamedBufferData(EBO, sizeof(indices), indices, GL_STATIC_DRAW);


		glVertexArrayElementBuffer(VAO, EBO);
		glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(Vertex));

		glEnableVertexArrayAttrib(VAO, 0);
		glEnableVertexArrayAttrib(VAO, 1);

		glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Vertex::pos));
		glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, Vertex::uv));

		glVertexArrayAttribBinding(VAO, 0, 0);
		glVertexArrayAttribBinding(VAO, 1, 0);

		load_shader("assets/shaders/default.vert", "assets/shaders/default.frag", &shader);
		load_texture("assets/images/uv_checker.png", &texture);

		glProgramUniform1i(shader, glGetUniformLocation(shader, "tex"), 0);
	}

	void update() {
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindTexture(GL_TEXTURE_2D, texture.id);

		glUseProgram(shader);
		glBindVertexArray(VAO);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
}

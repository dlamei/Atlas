#include "gl_utils.h"

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
			msg.resize(infoLogLength + 1);

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
				msg.resize(infoLogLength + 1);

				//TODO: error handling
				glGetProgramInfoLog(id, infoLogLength, nullptr, msg.data());
				CORE_WARN("Shader Linking:");
				CORE_WARN("{}", msg);
				return false;
			}
		}

		for (uint32_t i = 0; i < moduleCount; i++) glDetachShader(id, modules[i]);
		for (uint32_t i = 0; i < moduleCount; i++) glDeleteShader(modules[i]);

		*programID = id;
		return true;
	}

	uint32_t load_shader(const char *vertexFilePath, const char *fragmentFilePath) {

		uint32_t modules[2]{};
		load_shader_module("assets/shaders/default.vert", GL_VERTEX_SHADER, &modules[0]);
		uint32_t fragShaderID{};
		load_shader_module("assets/shaders/default.frag", GL_FRAGMENT_SHADER, &modules[1]);

		uint32_t id{};
		link_shader_modules(modules, 2, &id);
		return id;
	}

	uint32_t vertexArrayID;
	uint32_t VBO, VAO, EBO;
	uint32_t shader;

	void init()
	{
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
		glDebugMessageCallback(gl_debug_msg, 0);

		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

		float vertices[] =
		{
			-0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f, // Lower left corner
			0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f, // Lower right corner
			0.0f, 0.5f * float(sqrt(3)) * 2 / 3, 0.0f, // Upper corner
			-0.5f / 2, 0.5f * float(sqrt(3)) / 6, 0.0f, // Inner left
			0.5f / 2, 0.5f * float(sqrt(3)) / 6, 0.0f, // Inner right
			0.0f, -0.5f * float(sqrt(3)) / 3, 0.0f // Inner down
		};

		uint32_t indices[] =
		{
			0, 3, 5, // Lower left triangle
			3, 2, 4, // Lower right triangle
			5, 4, 1 // Upper triangle
		};

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		shader = load_shader("assets/shaders/default.vert", "assets/shaders/default.frag");
	}

	void update() {

		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shader);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);
	}

}

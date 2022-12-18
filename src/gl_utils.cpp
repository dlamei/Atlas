#include "gl_utils.h"

#include <glm/glm.hpp>
#include <stb_image.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <GLFW/glfw3.h>


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

	void create_texture2D(uint32_t width, uint32_t height, GLenum format, bool mipmap, uint32_t *texture) {
		uint32_t id;
		glCreateTextures(GL_TEXTURE_2D, 1, &id);
		glTextureStorage2D(id, 1, format, width, height);

		glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		if (mipmap) glGenerateTextureMipmap(id);

		*texture = id;
	}

	void set_texture2D_data(uint32_t texture, uint32_t width, uint32_t height, GLenum dataFormat, void *data) {
		if (!texture) {
			CORE_WARN("Texture2D: can not set data, Texture is not initialized!");
			return;
		}

		glTextureSubImage2D(texture, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, data);
	}

	bool load_texture2D(const char *filePath, uint32_t *texture) {
		int width, height, channels;

		stbi_set_flip_vertically_on_load(true);

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

		uint32_t tex{};
		create_texture2D(width, height, internalFormat, true, &tex);
		set_texture2D_data(tex, width, height, dataFormat, data);

		stbi_image_free(data);

		*texture = tex;

		return true;
	}

	void init_imgui(GLFWwindow *window) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO &io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImGui::StyleColorsDark();

		ImGuiStyle &style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

	}

	uint32_t vertexArrayID;
	uint32_t VBO, VAO, EBO, FBO;
	uint32_t shader;
	uint32_t texture;
	std::shared_ptr<GLTexture2D> testTexture;

	uint32_t textureColorbuffer;

	void init_opengl()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
		glDebugMessageCallback(gl_debug_msg, 0);
	}

	void init() {

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
		load_texture2D("assets/images/uv_checker.png", &texture);

		glProgramUniform1i(shader, glGetUniformLocation(shader, "tex"), 0);


		unsigned int framebuffer;
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		// generate texture
		create_texture2D(800, 600, GL_RGB8, false, &textureColorbuffer);

		// attach it to currently bound framebuffer object
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

		unsigned int rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);


		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void imgui_begin() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
	}

	void imgui_end() {
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		ImGuiIO &io = ImGui::GetIO();

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow *backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	void update() {
		imgui_begin();

		ImGui::ShowDemoWindow();

		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindTexture(GL_TEXTURE_2D, texture);

		glUseProgram(shader);
		glBindVertexArray(VAO);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		imgui_end();
	}

	GLTexture2D::GLTexture2D(const GLTexture2DCreateInfo &info)
		: m_Width(info.width), m_Height(info.height), m_Format(info.format), m_Mipmap(info.mipmap)
	{
		create_texture2D(info.width, info.height, info.format, info.mipmap, &m_ID);
	}

	GLTexture2D::~GLTexture2D()
	{
		glDeleteTextures(1, &m_ID);
	}

	void GLTexture2D::set_data(void *data, GLenum format)
	{
		set_texture2D_data(m_ID, m_Width, m_Height, format, data);
	}

	void GLTexture2D::bind()
	{
		glBindTexture(GL_TEXTURE_2D, m_ID);
	}

	GLBuffer::GLBuffer(size_t size)
		: m_Size(size)
	{
		glCreateBuffers(1, &m_ID);
		glNamedBufferData(m_ID, size, nullptr, GL_DYNAMIC_DRAW);
	}

	GLBuffer::GLBuffer(void *data, size_t size)
		: m_Size(size)
	{
		glCreateBuffers(1, &m_ID);
		glNamedBufferData(m_ID, size, data, GL_STATIC_DRAW);
	}

	void GLBuffer::set_data(void *data)
	{
		glNamedBufferSubData(m_ID, 0, m_Size, data);
	}

	void GLBuffer::set_sub_data(void *data, size_t size, size_t offset)
	{
		glNamedBufferSubData(m_ID, offset, size, data);
	}

	GLBuffer::~GLBuffer()
	{
		glDeleteBuffers(1, &m_ID);
	}
}

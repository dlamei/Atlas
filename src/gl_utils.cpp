#include "gl_utils.h"

#include "imgui_build.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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

	void create_texture2D(uint32_t width, uint32_t height, GLenum format, bool mipmap, GLenum minFilter, GLenum magFilter, uint32_t *texture) {
		uint32_t id;
		glCreateTextures(GL_TEXTURE_2D, 1, &id);
		glTextureStorage2D(id, 1, format, width, height);

		glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, minFilter);
		glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, magFilter);
		if (mipmap) glGenerateTextureMipmap(id);

		*texture = id;
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

	bool load_shader_module(const char *filePath, GLenum shaderType, uint32_t *shaderID) {

		uint32_t id = glCreateShader(shaderType);

		std::string shaderCode;
		std::ifstream file(filePath, std::ios::in);

		if (!file.is_open()) {
			CORE_WARN("Could not find file: {}", filePath);
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

	void reflect_shader(uint32_t program, GLShaderReflectionData *data)
	{
		*data = GLShaderReflectionData{};
		std::string buffer;
		int nameLength;
		glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &nameLength);
		buffer.resize(nameLength);
		int count;

		//uniform
		glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
		for (int i = 0; i < count; i++) {
			GLenum type;
			int nameLen, size, location;
			glGetActiveUniform(program, (uint32_t)i, (int)buffer.size(), &nameLen, &size, &type, buffer.data());
			if (type == GL_UNIFORM_BLOCK) continue;
			std::string name(buffer.data(), nameLen);
			location = glGetUniformLocation(program, name.c_str());
			if (location == -1) continue;

			GLUniformInfo info{};
			info.location = location;
			info.type = type;
			info.size;

			data->uniforms.insert({ name, info });
			//CORE_TRACE("Uniform #{} name: {} type: 0x{:x} size: {} location: {}", i, name, type, size, location);
		}

		//uniform blocks
		glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &nameLength);
		buffer.resize(nameLength);

		glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &count);
		for (int i = 0; i < count; i++) {
			int binding, dataSize, nameLen;
			glGetActiveUniformBlockName(program, (uint32_t)i, (int)buffer.size(), &nameLen, buffer.data());
			glGetActiveUniformBlockiv(program, (uint32_t)i, GL_UNIFORM_BLOCK_BINDING, &binding);
			glGetActiveUniformBlockiv(program, (uint32_t)i, GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize);
			std::string name(buffer.data(), nameLen);

			int indx = glGetUniformBlockIndex(program, name.c_str());

			GLUniformBlockInfo info{};
			info.binding = binding;
			info.size = dataSize;
			info.index = indx;

			data->unifromBlocks.insert({ name, info });
			//CORE_TRACE("Uniform Block #{} name: {} binding: {} dataSize: {}", i, name, binding, dataSize);
		}

	}

	void resize_viewport(uint32_t width, uint32_t height)
	{
		glViewport(0, 0, width, height);
	}


	GLTexture2D::GLTexture2D(const GLTexture2DCreateInfo &info)
		: m_Width(info.width), m_Height(info.height), m_Format(info.format), m_Mipmap(info.mipmap)
	{
		CORE_ASSERT(m_Width, "GLTexture2D::GLTexture2D: width is zero");
		CORE_ASSERT(m_Height, "GLTexture2D::GLTexture2D: height is zero");

		create_texture2D(info.width, info.height, info.format, info.mipmap, info.minFilter, info.magFilter, &m_ID);
	}

	GLTexture2D::~GLTexture2D()
	{
		glDeleteTextures(1, &m_ID);
	}

	void GLTexture2D::set_data(void *data, GLenum format)
	{
		set_texture2D_data(m_ID, m_Width, m_Height, format, data);
	}

	void GLTexture2D::bind(uint32_t indx)
	{
		glActiveTexture(GL_TEXTURE0 + indx);
		glBindTexture(GL_TEXTURE_2D, m_ID);
	}

	Ref<GLTexture2D> load_texture2D(const char *filePath) {
		int width, height, channels;

		stbi_set_flip_vertically_on_load(true);

		stbi_uc *data = nullptr;
		data = stbi_load(filePath, &width, &height, &channels, 0);

		if (!data || stbi_failure_reason()) {
			CORE_WARN("Failed to load image: {}", filePath);
			CORE_WARN("{}", stbi_failure_reason());
			return nullptr;
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

		GLTexture2DCreateInfo info{};
		info.format = internalFormat;
		info.height = height;
		info.width = width;
		info.mipmap = true;
		info.minFilter = GL_LINEAR_MIPMAP_LINEAR;
		info.magFilter = GL_LINEAR;

		Ref<GLTexture2D> tex = make_ref<GLTexture2D>(info);
		tex->set_data(data, dataFormat);

		stbi_image_free(data);

		return tex;
	}

	GLBuffer::GLBuffer(const GLBufferCreateInfo &info)
		:m_Size(info.size)
	{
		glCreateBuffers(1, &m_ID);

		if (!info.data.second || !info.data.first) {
			glNamedBufferData(m_ID, m_Size, nullptr, info.usage);
			return;
		}

		if (info.data.second > m_Size) {
			CORE_WARN("GLBuffer::GLBuffer: size of the data has to be smaller or equal to the size of the buffer");
		}

		if (info.data.second == m_Size) {
			glNamedBufferData(m_ID, m_Size, info.data.first, info.usage);
		}
		else {
			glNamedBufferSubData(m_ID, 0, info.data.second, info.data.first);
		}
	}

	void GLBuffer::set_data(void *data, size_t size)
	{
		CORE_ASSERT(size <= m_Size, "GLBuffer::set_data error: size has to be smaller or equal than the size of the buffer");
		glNamedBufferSubData(m_ID, 0, m_Size, data);
	}

	GLBuffer::~GLBuffer()
	{
		glDeleteBuffers(1, &m_ID);
	}

	void bind_vertex_buffer(Ref<GLBuffer> GLBuffer, size_t stride, uint32_t indx, uint32_t offset) {
		glBindVertexBuffer(indx, GLBuffer->id(), offset, (GLsizei)stride);
	}

	void bind_index_buffer(Ref<GLBuffer> buffer) {
		glBindBuffer(GL_INDEX_BUFFER, buffer->id());
	}

	GLRenderbuffer::GLRenderbuffer(GLRenderbufferCreateInfo &info)
		:m_Width(info.width), m_Height(info.height), m_Format(info.format)
	{
		glCreateRenderbuffers(1, &m_RBO);
		glNamedRenderbufferStorage(m_RBO, m_Format, m_Width, m_Height);
	}

	GLRenderbuffer::~GLRenderbuffer()
	{
		glDeleteRenderbuffers(1, &m_RBO);
	}

	GLFramebuffer::GLFramebuffer()
	{
		glCreateFramebuffers(1, &m_FBO);
	}

	GLFramebuffer::~GLFramebuffer()
	{
		glDeleteFramebuffers(1, &m_FBO);
	}

	void GLFramebuffer::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	}

	void GLFramebuffer::push_tex_attachment(GLenum attachment, uint32_t texID)
	{
		glNamedFramebufferTexture(m_FBO, attachment, texID, 0);
	}

	void GLFramebuffer::push_rbo_attachment(GLenum attachment, uint32_t rbo)
	{
		glNamedFramebufferRenderbuffer(m_FBO, attachment, GL_RENDERBUFFER, rbo);
	}

	bool GLFramebuffer::check_status()
	{
		return (glCheckNamedFramebufferStatus(m_FBO, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}

	GLenum GLFramebuffer::get_status()
	{
		return glCheckNamedFramebufferStatus(m_FBO, GL_FRAMEBUFFER);
	}

	GLShader::GLShader(const GLShaderCreateInfo &info)
	{
		std::vector<uint32_t> modules;
		modules.resize(info.size());

		for (uint32_t i = 0; i < (uint32_t)info.size(); i++) {
			auto &pair = info.at(i);
			if (!load_shader_module(pair.first.c_str(), pair.second, &modules.at(i))) {
				CORE_WARN("GLShader::GLShader: error while compiling module: {}", pair.first);
				return;
			}
		}

		if (!link_shader_modules(modules.data(), (uint32_t)modules.size(), &m_ID)) {
			CORE_WARN("GLShader::GLShader: error while compiling shader");
		}

		for (auto i : modules) glDeleteShader(i);

		reflect_shader(m_ID, &m_ReflectionData);

		for (auto &block : m_ReflectionData.unifromBlocks) {
			if (block.second.binding == 0) block.second.binding = block.second.index;
			glUniformBlockBinding(m_ID, block.second.index, block.second.binding);
		}
	}

	GLShader::~GLShader()
	{
		glDeleteProgram(m_ID);
	}

	void GLShader::bind()
	{
		glUseProgram(m_ID);
	}

	void GLShader::set_int(const std::string &name, int32_t value)
	{
		int location = get_uniform_location(name);
		glProgramUniform1i(m_ID, location, value);
	}

	void GLShader::set_int2(const std::string &name, const glm::ivec2 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniform2i(m_ID, location, value.x, value.y);
	}

	void GLShader::set_int3(const std::string &name, const glm::ivec3 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniform3i(m_ID, location, value.x, value.y, value.z);
	}

	void GLShader::set_int4(const std::string &name, const glm::ivec4 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniform4i(m_ID, location, value.x, value.y, value.z, value.w);
	}

	void GLShader::set_int_vec(const std::string &name, int32_t *data, size_t count)
	{
		int location = get_uniform_location(name);
		glProgramUniform1iv(m_ID, location, (int)count, data);
	}

	void GLShader::set_uint(const std::string &name, uint32_t value)
	{
		int location = get_uniform_location(name);
		glProgramUniform1ui(m_ID, location, value);
	}

	void GLShader::set_uint2(const std::string &name, const glm::uvec2 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniform2ui(m_ID, location, value.x, value.y);
	}

	void GLShader::set_uint3(const std::string &name, const glm::uvec3 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniform3ui(m_ID, location, value.x, value.y, value.z);
	}

	void GLShader::set_uint4(const std::string &name, const glm::uvec4 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniform4ui(m_ID, location, value.x, value.y, value.z, value.w);
	}

	void GLShader::set_uint_vec(const std::string &name, uint32_t *data, size_t count)
	{
		int location = get_uniform_location(name);
		glProgramUniform1uiv(m_ID, location, (int)count, data);
	}

	void GLShader::set_float(const std::string &name, float value)
	{
		int location = get_uniform_location(name);
		glProgramUniform1f(m_ID, location, value);
	}

	void GLShader::set_float2(const std::string &name, const glm::vec2 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniform2f(m_ID, location, value.x, value.y);
	}

	void GLShader::set_float3(const std::string &name, const glm::vec3 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniform3f(m_ID, location, value.x, value.y, value.z);
	}

	void GLShader::set_float4(const std::string &name, const glm::vec4 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniform4f(m_ID, location, value.x, value.y, value.z, value.w);
	}

	void GLShader::set_float_vec(const std::string &name, float *data, size_t count)
	{
		int location = get_uniform_location(name);
		glProgramUniform1fv(m_ID, location, (int)count, data);
	}

	void GLShader::set_mat3(const std::string &name, const glm::mat3 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniformMatrix3fv(m_ID, location, 1, false, glm::value_ptr(value));
	}

	void GLShader::set_mat4(const std::string &name, const glm::mat4 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniformMatrix4fv(m_ID, location, 1, false, glm::value_ptr(value));
	}

	GLUniformInfo *GLShader::get_uniform_info(const std::string &name)
	{
		if (m_ReflectionData.uniforms.find(name) == m_ReflectionData.uniforms.end()) {
			return nullptr;
		}

		return &m_ReflectionData.uniforms.find(name)->second;
	}

	int GLShader::get_uniform_location(const std::string &name)
	{
		if (m_ReflectionData.uniforms.find(name) == m_ReflectionData.uniforms.end()) {
			CORE_WARN("GLShader::get_uniform_location: could not find uniform: {}", name);
			return -1;
		}

		return m_ReflectionData.uniforms.find(name)->second.location;
	}

	int GLShader::get_block_binding(const std::string &name)
	{
		if (m_ReflectionData.unifromBlocks.find(name) == m_ReflectionData.unifromBlocks.end()) {
			CORE_WARN("GLShader::get_block_binding: could not find uniform: {}", name);
			return -1;
		}

		return m_ReflectionData.unifromBlocks.find(name)->second.binding;
	}

	VertexLayout::VertexLayout()
	{
		glCreateVertexArrays(1, &m_VAO);
	}

	VertexLayout::~VertexLayout()
	{
		glDeleteVertexArrays(1, &m_VAO);
	}

	void VertexLayout::push_attrib(uint32_t count, GLenum type, uint32_t offset, uint32_t bufferIndx)
	{
		glEnableVertexArrayAttrib(m_VAO, m_AttribIndx);
		glVertexArrayAttribFormat(m_VAO, m_AttribIndx, 3, GL_FLOAT, false, offsetof(Vertex, Vertex::pos));
		glVertexArrayAttribBinding(m_VAO, m_AttribIndx, bufferIndx);
		m_AttribIndx++;
	}

	void VertexLayout::bind()
	{
		glBindVertexArray(m_VAO);
	}

	void gl_debug_msg(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
		const GLchar *message, const void *userParam)
	{
		std::string sType = "";

		if (type == GL_DEBUG_TYPE_ERROR) sType = "ERROR";
		else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR) sType = "DEPRECATED";
		else return;

		if (severity == GL_DEBUG_SEVERITY_LOW) {
			CORE_TRACE("OpenGL {}: {}", sType, gl_get_error_string(type));
			CORE_TRACE("{}", message);
		}
		else if (severity == GL_DEBUG_SEVERITY_MEDIUM) {
			CORE_WARN("OpenGL {}: {}", sType, gl_get_error_string(type));
			CORE_WARN("{}", message);
		}
		else if (severity == GL_DEBUG_SEVERITY_HIGH) {
			CORE_ERROR("OpenGL {}: {}", sType, gl_get_error_string(type));
			CORE_ERROR("{}", message);
		}
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

	void init_opengl()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
		glDebugMessageCallback(gl_debug_msg, 0);

		CORE_TRACE("OpenGL Info:");
		CORE_TRACE(" vendor:	{}", (const char *)glGetString(GL_VENDOR));
		CORE_TRACE(" renderer:	{}", (const char *)glGetString(GL_RENDERER));
		CORE_TRACE(" version:	{}", (const char *)glGetString(GL_VERSION));
		CORE_TRACE("");
	}

	Ref<VertexLayout> VAO;
	Ref<GLBuffer> VBO, IBO, cameraBuffer;
	Ref<GLShader> shader;
	Ref<GLTexture2D> texture;
	//Ref<GLRenderbuffer> depthBuffer;

	void init() {

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

		{
			GLBufferCreateInfo info{};
			info.size = sizeof(vertices);
			info.data = { (void *)vertices, sizeof(vertices) };
			info.usage = GL_STATIC_DRAW;
			VBO = make_ref<GLBuffer>(info);
		}

		{
			GLBufferCreateInfo info{};
			info.size = sizeof(indices);
			info.data = { (void *)indices, sizeof(indices) };
			info.usage = GL_STATIC_DRAW;
			IBO = make_ref<GLBuffer>(info);
		}

		VAO = make_ref<VertexLayout>();
		VAO->push_attrib(3, GL_FLOAT, offsetof(Vertex, Vertex::pos));
		VAO->push_attrib(2, GL_FLOAT, offsetof(Vertex, Vertex::uv));

		{
			GLShaderCreateInfo info = {
				{"assets/shaders/default.vert", GL_VERTEX_SHADER},
				{"assets/shaders/default.frag", GL_FRAGMENT_SHADER}
			};
			shader = make_ref<GLShader>(info);
			shader->set_int("tex", 0);

			glm::mat4 camera = glm::ortho(-1, 1, -1, 1);


			{
				GLBufferCreateInfo info{};
				info.size = sizeof(glm::mat4);
				info.data = { &camera, sizeof(glm::mat4) };
				info.usage = GL_STATIC_DRAW;
				cameraBuffer = make_ref<GLBuffer>(info);
			}

			glBindBufferRange(GL_UNIFORM_BUFFER, shader->get_block_binding("CameraBuffer"), cameraBuffer->id(), 0, sizeof(glm::mat4));
		}

		texture = load_texture2D("assets/images/uv_checker.png");

		//{
		//	GLRenderbufferCreateInfo info{};
		//	info.width = 900;
		//	info.height = 900;
		//	info.format = GL_DEPTH24_STENCIL8;
		//	depthBuffer = make_ref<GLRenderbuffer>(info);
		//}

		//FBO = make_ref<GLFramebuffer>();
		//FBO->push_tex_attachment(GL_COLOR_ATTACHMENT0, colorBuffer->id());
		//FBO->push_rbo_attachment(GL_DEPTH_STENCIL_ATTACHMENT, depthBuffer->id());
	}

	void update() {

		//FBO->bind();
		//glViewport(0, 0, colorBuffer->width(), colorBuffer->height());

		//ImGui::ShowDemoWindow();

		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		texture->bind();

		shader->bind();
		VAO->bind();

		bind_vertex_buffer(VBO, sizeof(Vertex));
		bind_index_buffer(IBO);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		//ImGui::Begin("Viewport");
		//ImGui::PopStyleVar();
		//ImGui::BeginChild("Viewport");

		//ImVec2 wSize = ImGui::GetWindowSize();
		//ImGui::Image((void *)colorBuffer->id(), wSize, { 0, 1 }, { 1, 0 });

		//ImGui::EndChild();
		//ImGui::End();

	}

}

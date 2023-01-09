#include "gl_utils.h"

#include "imgui_build.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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

namespace gl_utils {

	static uint32_t s_GlobalVAO{ 0 };

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

	void set_texture2D_data(uint32_t texture, uint32_t width, uint32_t height, GLenum dataFormat, const void *data) {
		if (!texture) {
			CORE_WARN("Texture2D: can not set data, Texture is not initialized!");
			return;
		}

		glTextureSubImage2D(texture, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, data);
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

		//uniform
		{
			int count{ 0 };
			std::string buffer;
			int buffSize{ 0 };

			glGetProgramInterfaceiv(program, GL_UNIFORM, GL_MAX_NAME_LENGTH, &buffSize);
			buffer.resize(buffSize);

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
			}
		}

		//uniform blocks
		{
			int count{ 0 };
			std::string buffer;
			int buffSize{ 0 };

			glGetProgramInterfaceiv(program, GL_UNIFORM_BLOCK, GL_MAX_NAME_LENGTH, &buffSize);
			buffer.resize(buffSize);

			glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &count);
			for (int i = 0; i < count; i++) {
				int binding = 0, dataSize = 0, nameLen = 0;
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
			}
		}

		//storage buffers
		{
			int count{ 0 };
			std::string buffer;
			int buffSize{ 0 };

			glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &buffSize);
			buffer.resize(buffSize);

			glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &count);

			for (int i = 0; i < count; i++) {
				int nameLength = 0;
				glGetProgramResourceName(program, GL_SHADER_STORAGE_BLOCK, i, buffSize, &nameLength, buffer.data());
				std::string name(buffer.data(), nameLength);

				int binding = 0;
				GLenum prop = GL_BUFFER_BINDING;
				glGetProgramResourceiv(program, GL_SHADER_STORAGE_BLOCK, i, 1, &prop, 1, nullptr, &binding);

				int size = 0;
				prop = GL_BUFFER_DATA_SIZE;
				glGetProgramResourceiv(program, GL_SHADER_STORAGE_BLOCK, i, 1, &prop, 1, nullptr, &size);

				int index = glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, name.data());

				GLStorageBlockInfo info{};
				info.size = size;
				info.binding = binding;
				info.index = index;

				data->storageBlocks.insert({ name, info });
			}
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

	void GLTexture2D::set_data(const void *data, GLenum format)
	{
		set_texture2D_data(m_ID, m_Width, m_Height, format, data);
	}

	void GLTexture2D::bind(uint32_t indx)
	{
		glActiveTexture(GL_TEXTURE0 + indx);
		glBindTexture(GL_TEXTURE_2D, m_ID);
	}

	GLBuffer::GLBuffer(const GLBufferCreateInfo &info)
		:m_Size(info.size)
	{
		glCreateBuffers(1, &m_ID);

		if (info.data == nullptr) {
			glNamedBufferData(m_ID, m_Size, nullptr, info.usage);
			return;
		}
		else {
			glNamedBufferData(m_ID, m_Size, info.data, info.usage);
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

	void bind_uniform_buffer(const Ref<GLBuffer> &buffer, uint32_t blockBinding, uint32_t offset)
	{
		glBindBufferRange(GL_UNIFORM_BUFFER, blockBinding, buffer->id(), offset, buffer->size());
	}

	void bind_shader(Ref<GLShader> shader)
	{
		glUseProgram(shader->id());
	}

	void bind_vertex_buffer(const Ref<GLBuffer> &GLBuffer, size_t stride, uint32_t indx, uint32_t offset) {
		glBindVertexBuffer(indx, GLBuffer->id(), offset, (GLsizei)stride);
	}

	void bind_index_buffer(const Ref<GLBuffer> &buffer) {
		glBindBuffer(GL_INDEX_BUFFER, buffer->id());
	}

	void bind_storage_buffer(const Ref<GLBuffer> &buffer, uint32_t blockBinding, uint32_t offset)
	{
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, blockBinding, buffer->id(), offset, buffer->size());
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

		for (auto &block : m_ReflectionData.storageBlocks) {
			if (block.second.binding == 0) block.second.binding = block.second.index;
			glShaderStorageBlockBinding(m_ID, block.second.index, block.second.binding);
		}
	}

	GLShader::~GLShader()
	{
		glDeleteProgram(m_ID);
	}

	void GLShader::set_int(const char *name, int32_t value)
	{
		int location = get_uniform_location(name);
		glProgramUniform1i(m_ID, location, value);
	}

	void GLShader::set_int2(const char *name, const glm::ivec2 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniform2i(m_ID, location, value.x, value.y);
	}

	void GLShader::set_int3(const char *name, const glm::ivec3 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniform3i(m_ID, location, value.x, value.y, value.z);
	}

	void GLShader::set_int4(const char *name, const glm::ivec4 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniform4i(m_ID, location, value.x, value.y, value.z, value.w);
	}

	void GLShader::set_int_vec(const char *name, int32_t *data, size_t count)
	{
		int location = get_uniform_location(name);
		glProgramUniform1iv(m_ID, location, (int)count, data);
	}

	void GLShader::set_uint(const char *name, uint32_t value)
	{
		int location = get_uniform_location(name);
		glProgramUniform1ui(m_ID, location, value);
	}

	void GLShader::set_uint2(const char *name, const glm::uvec2 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniform2ui(m_ID, location, value.x, value.y);
	}

	void GLShader::set_uint3(const char *name, const glm::uvec3 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniform3ui(m_ID, location, value.x, value.y, value.z);
	}

	void GLShader::set_uint4(const char *name, const glm::uvec4 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniform4ui(m_ID, location, value.x, value.y, value.z, value.w);
	}

	void GLShader::set_uint_vec(const char *name, uint32_t *data, size_t count)
	{
		int location = get_uniform_location(name);
		glProgramUniform1uiv(m_ID, location, (int)count, data);
	}

	void GLShader::set_float(const char *name, float value)
	{
		int location = get_uniform_location(name);
		glProgramUniform1f(m_ID, location, value);
	}

	void GLShader::set_float2(const char *name, const glm::vec2 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniform2f(m_ID, location, value.x, value.y);
	}

	void GLShader::set_float3(const char *name, const glm::vec3 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniform3f(m_ID, location, value.x, value.y, value.z);
	}

	void GLShader::set_float4(const char *name, const glm::vec4 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniform4f(m_ID, location, value.x, value.y, value.z, value.w);
	}

	void GLShader::set_float_vec(const char *name, float *data, size_t count)
	{
		int location = get_uniform_location(name);
		glProgramUniform1fv(m_ID, location, (int)count, data);
	}

	void GLShader::set_mat3(const char *name, const glm::mat3 &value)
	{
		int location = get_uniform_location(name);
		glProgramUniformMatrix3fv(m_ID, location, 1, false, glm::value_ptr(value));
	}

	void GLShader::set_mat4(const char *name, const glm::mat4 &value)
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

	int GLShader::get_uniform_block_binding(const std::string &name)
	{
		const auto &it = m_ReflectionData.unifromBlocks.find(name);
		if (it == m_ReflectionData.unifromBlocks.end()) {
			return -1;
		}

		return it->second.binding;
	}

	int GLShader::get_storage_block_binding(const std::string &name)
	{
		const auto it = m_ReflectionData.storageBlocks.find(name);
		if (it == m_ReflectionData.storageBlocks.end()) {
			return -1;
		}

		return it->second.binding;
	}

	GLVertexLayout::GLVertexLayout()
	{
		//glCreateVertexArrays(1, &s_GlobalVAO);
	}

	GLVertexLayout::~GLVertexLayout()
	{
		//glDeleteVertexArrays(1, &s_GlobalVAO);
	}

	void GLVertexLayout::push_attrib(uint32_t count, GLenum type, uint32_t offset, uint32_t bufferIndx)
	{
		//glEnableVertexArrayAttrib(s_GlobalVAO, m_AttribIndx);
		//glVertexArrayAttribFormat(s_GlobalVAO, m_AttribIndx, count, type, false, offset);
		//glVertexArrayAttribBinding(s_GlobalVAO, m_AttribIndx, bufferIndx);
		AttribInfo info{};
		info.count = count;
		info.type = type;
		info.offset = offset;
		info.bufferIndex = bufferIndx;
		info.attribIndex = m_AttribIndx;
		m_Attributes.push_back(info);

		m_AttribIndx++;
	}

	void GLVertexLayout::bind()
	{
		//GLint maxAttribs;
		//glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);

		//for (GLint i = 0; i < maxAttribs; i++) {
		//	glDisableVertexArrayAttrib(s_GlobalVAO, i);
		//}

		//for (auto &attrib : m_Attributes) {
		//	glEnableVertexArrayAttrib(s_GlobalVAO, attrib.attribIndex);
		//	glVertexArrayAttribFormat(s_GlobalVAO, attrib.attribIndex, attrib.count, attrib.type, false, attrib.offset);
		//	glVertexArrayAttribBinding(s_GlobalVAO, attrib.attribIndex, attrib.bufferIndex);
		//}
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

	void init_opengl()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
		glDebugMessageCallback(gl_debug_msg, 0);

		struct Vertex {
			glm::vec3 pos;
			glm::vec2 uv;
			glm::vec4 color;
			int texID;
		};


		glCreateVertexArrays(1, &s_GlobalVAO);
		glBindVertexArray(s_GlobalVAO);

		glEnableVertexArrayAttrib(s_GlobalVAO, 0);
		glVertexArrayAttribFormat(s_GlobalVAO, 0, 3, GL_FLOAT, false, offsetof(Vertex, pos));
		glVertexArrayAttribBinding(s_GlobalVAO, 0, 0);

		glEnableVertexArrayAttrib(s_GlobalVAO, 1);
		glVertexArrayAttribFormat(s_GlobalVAO, 1, 2, GL_FLOAT, false, offsetof(Vertex, uv));
		glVertexArrayAttribBinding(s_GlobalVAO, 1, 0);

		glEnableVertexArrayAttrib(s_GlobalVAO, 2);
		glVertexArrayAttribFormat(s_GlobalVAO, 2, 4, GL_FLOAT, false, offsetof(Vertex, color));
		glVertexArrayAttribBinding(s_GlobalVAO, 2, 0);

		glEnableVertexArrayAttrib(s_GlobalVAO, 3);
		glVertexArrayAttribFormat(s_GlobalVAO, 3, 1, GL_INT, false, offsetof(Vertex, texID));
		glVertexArrayAttribBinding(s_GlobalVAO, 3, 0);

		CORE_TRACE("OpenGL Info:");
		CORE_TRACE(" vendor:	{}", (const char *)glGetString(GL_VENDOR));
		CORE_TRACE(" renderer:	{}", (const char *)glGetString(GL_RENDERER));
		CORE_TRACE(" version:	{}\n", (const char *)glGetString(GL_VERSION));
	}
}

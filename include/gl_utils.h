#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

struct GLFWwindow;

#define GL_VERTEX_BUFFER GL_ARRAY_BUFFER
#define GL_INDEX_BUFFER GL_ELEMENT_ARRAY_BUFFER

namespace gl_utils {

	class GLTexture2D;
	class GLBuffer;
	class GLShader;
	class GLRenderbuffer;
	class GLFramebuffer;

	struct GLUniformInfo {
		GLenum type;
		int location;
		int size;
	};

	struct GLUniformBlockInfo {
		int binding;
		int index;
		int size;
	};

	struct GLStorageBlockInfo {
		int size;
		int index;
		int binding;
	};

	struct GLShaderReflectionData {
		std::unordered_map<std::string, GLUniformInfo> uniforms;
		std::unordered_map<std::string, GLUniformBlockInfo> unifromBlocks;
		std::unordered_map<std::string, GLStorageBlockInfo> storageBlocks;
	};

	void create_texture2D(uint32_t width, uint32_t height, GLenum format, bool mipmap, uint32_t *texture);
	void set_texture2D_data(uint32_t texture, uint32_t width, uint32_t height, GLenum dataFormat, void *data);
	bool load_texture2D(const char *filePath, uint32_t *texture);

	bool load_shader_module(const char *filePath, GLenum shaderType, uint32_t *shaderID);
	bool link_shader_modules(uint32_t *modules, uint32_t moduleCount, uint32_t *programID);
	void reflect_shader(uint32_t program, GLShaderReflectionData *data);

	void resize_viewport(uint32_t width, uint32_t height);

	void init_opengl();

	struct GLTexture2DCreateInfo {
		uint32_t width;
		uint32_t height;
		GLenum format;

		GLenum minFilter;
		GLenum magFilter;

		bool mipmap;
	};

	class GLTexture2D {
	public:

		GLTexture2D(const GLTexture2DCreateInfo &info);
		GLTexture2D(const GLTexture2D &) = delete;
		~GLTexture2D();

		void set_data(void *data, GLenum format);
		void bind(uint32_t indx = 0);

		inline uint32_t id() { return m_ID; }
		inline uint32_t width() { return m_Width; }
		inline uint32_t height() { return m_Height; }
		inline bool has_mipmap() { return m_Mipmap; }

	private:
		uint32_t m_ID{ 0 };
		uint32_t m_Width;
		uint32_t m_Height;
		GLenum m_Format;
		bool m_Mipmap;
	};

	struct GLBufferCreateInfo {
		GLenum usage;
		size_t size;
		void *data;
		//std::pair<void *, size_t> data{ nullptr, 0 }; //ptr to data, size
	};

	class GLBuffer {
	public:

		GLBuffer(const GLBufferCreateInfo &info);
		GLBuffer(const GLBuffer &) = delete;
		~GLBuffer();

		void set_data(void *data, size_t size);

		inline size_t size() { return m_Size; }
		inline uint32_t id() { return m_ID; }

	private:
		size_t m_Size;
		uint32_t m_ID{ 0 };
	};

	void bind_uniform_buffer(const Ref<GLBuffer> &buffer, uint32_t blockBinding, uint32_t offset = 0);
	void bind_vertex_buffer(const Ref<GLBuffer> &GLBuffer, size_t stride, uint32_t indx = 0, uint32_t offset = 0);
	void bind_index_buffer(const Ref<GLBuffer> &buffer);
	void bind_storage_buffer(const Ref<GLBuffer> &buffer, uint32_t blockBinding, uint32_t offset = 0);

	using GLShaderCreateInfo = std::vector<std::pair<std::string, GLenum>>;

	class GLShader {
	public:

		GLShader(const GLShaderCreateInfo &info);
		GLShader(const GLShader &) = delete;
		~GLShader();

		//void bind();

		void set_int(const char *name, int32_t value);
		void set_int2(const char *name, const glm::ivec2 &value);
		void set_int3(const char *name, const glm::ivec3 &value);
		void set_int4(const char *name, const glm::ivec4 &value);
		void set_int_vec(const char *name, int32_t *data, size_t count);

		void set_uint(const char *name, uint32_t value);
		void set_uint2(const char *name, const glm::uvec2 &value);
		void set_uint3(const char *name, const glm::uvec3 &value);
		void set_uint4(const char *name, const glm::uvec4 &value);
		void set_uint_vec(const char *name, uint32_t *data, size_t count);

		void set_float(const char *name, float value);
		void set_float2(const char *name, const glm::vec2 &value);
		void set_float3(const char *name, const glm::vec3 &value);
		void set_float4(const char *name, const glm::vec4 &value);
		void set_float_vec(const char *name, float *data, size_t count);

		void set_mat3(const char *name, const glm::mat3 &value);
		void set_mat4(const char *name, const glm::mat4 &value);

		GLUniformInfo *get_uniform_info(const std::string &name);
		int get_uniform_location(const std::string &name);
		int get_uniform_block_binding(const std::string &name);
		int get_storage_block_binding(const std::string &name);

		inline uint32_t id() { return m_ID; }

	private:
		uint32_t m_ID{ 0 };
		GLShaderReflectionData m_ReflectionData;
		//std::unordered_map<std::string, int> m_UniformCache;
	};

	void bind_shader(Ref<GLShader> shader);

	struct GLRenderbufferCreateInfo {
		uint32_t width;
		uint32_t height;
		GLenum format;
	};

	class GLRenderbuffer {
	public:

		GLRenderbuffer(GLRenderbufferCreateInfo &info);
		GLRenderbuffer(const GLRenderbuffer &) = delete;
		~GLRenderbuffer();

		inline uint32_t id() { return m_RBO; }

	private:
		uint32_t m_RBO;
		uint32_t m_Width;
		uint32_t m_Height;
		GLenum m_Format;
	};

	class GLFramebuffer {
	public:

		GLFramebuffer();
		GLFramebuffer(const GLFramebuffer &) = delete;
		~GLFramebuffer();

		void bind();

		void push_tex_attachment(GLenum attachment, uint32_t texID);
		void push_rbo_attachment(GLenum attachment, uint32_t rbo);

		bool check_status();
		GLenum get_status();

	private:
		uint32_t m_FBO;
		uint32_t m_ColAttachmentIndx{ 0 };
	};

	class GLVertexLayout {
	public:

		GLVertexLayout();
		GLVertexLayout(const GLVertexLayout &) = delete;
		~GLVertexLayout();

		void push_attrib(uint32_t count, GLenum type, uint32_t offset, uint32_t bufferIndx = 0);
		void bind();

	private:
		uint32_t m_VAO{ 0 };
		uint32_t m_AttribIndx{ 0 };
	};
}
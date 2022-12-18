#pragma once
#include <glad/glad.h>

struct GLFWwindow;

namespace gl_utils {

	void init_opengl();

	struct GLTexture2DCreateInfo {
		uint32_t width;
		uint32_t height;
		GLenum format;

		bool mipmap;
	};

	class GLTexture2D {

		GLTexture2D(const GLTexture2DCreateInfo &info);
		GLTexture2D(const GLTexture2D &) = delete;

		~GLTexture2D();

		void set_data(void *data, GLenum size);
		void bind();

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

	class GLBuffer {

		~GLBuffer();

		GLBuffer(size_t size);
		GLBuffer(void *data, size_t offset = 0);

		void set_data(void *data);
		void set_sub_data(void *data, size_t size, size_t offset = 0);

		inline size_t size() { return m_Size; }
		inline uint32_t id() { return m_ID; }

	private:
		size_t m_Size;
		uint32_t m_ID{ 0 };
	};

	void init();
	void update();

	void init_imgui(GLFWwindow *window);
}
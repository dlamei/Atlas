#pragma once

#include "imgui.h"

#include <glm/glm.hpp>

namespace gl_utils {
	class GLTexture2D;
	class GLFramebuffer;
}

namespace Atlas {
	class Texture2D;
	class FrameBuffer;
}

namespace ImGui {

	void Image(const Atlas::Texture2D &texture, const ImVec2 &size,
		const ImVec2 &uv0 = ImVec2(0, 1), const ImVec2 &uv1 = ImVec2(1, 0),
		const ImVec4 &tint_col = ImVec4(1, 1, 1, 1), const ImVec4 &border_col = ImVec4(0, 0, 0, 0));

}

namespace Atlas {

	class Color {
	public:
		Color();
		Color(uint8_t value);
		Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
		Color(uint8_t r, uint8_t g, uint8_t b);

		inline uint8_t red() const;
		inline uint8_t green() const;
		inline uint8_t blue() const;
		inline uint8_t alpha() const;

		glm::vec4 normalized_vec();

		explicit operator uint32_t() const;

	private:
		uint32_t m_Data;
	};

	std::ostream &operator<<(std::ostream &os, const Color &c);

	enum class ColorFormat : uint32_t {
		R8G8B8A8,
		R8G8B8,
		D32,
		D24S8,
	};

	enum class TextureFilter : uint32_t {
		LINEAR = 0,
		NEAREST,
	};

	struct Texture2DCreateInfo {
		uint32_t width;
		uint32_t height;
		ColorFormat format;
		TextureFilter filter;
		bool mipmap;
	};

	class Texture2D {
	public:

		Texture2D() = default;
		Texture2D(const Texture2DCreateInfo &info);

		static Texture2D color(uint32_t width, uint32_t height, TextureFilter filter = TextureFilter::LINEAR);
		static std::optional<Texture2D> load(const char *filePath, TextureFilter filter = TextureFilter::LINEAR);

		inline bool is_init() { return m_Texture != nullptr; }
		inline ColorFormat format() { return m_Format; }

		void set_data(Color *data);
		void bind(uint32_t indx = 0);

		uint32_t width();
		uint32_t height();
		bool has_mipmap();
		uint32_t get_native() const;


	private:
		ColorFormat m_Format{ 0 };
		Ref<gl_utils::GLTexture2D> m_Texture{ nullptr };
	};


	class FrameBuffer {
	public:

		using Attachment = Texture2D;

		FrameBuffer() = default;
		FrameBuffer(std::vector<Attachment> colorAttachments);

		void bind();
		static void unbind();
		void set_color_attachment(Texture2D &texture, uint32_t index);
		void set_depth_stencil_texture(Texture2D &texture);

		const Texture2D &get_color_attachment(uint32_t index);

		inline uint32_t width() { return m_Width; }
		inline uint32_t height() { return m_Height; }

		inline bool is_init() { return m_Framebuffer != nullptr; }

	private:
		Ref<gl_utils::GLFramebuffer> m_Framebuffer{ nullptr };
		std::unordered_map<uint32_t, Texture2D> m_ColorTextures;
		Texture2D m_DepthStencilTexture;

		uint32_t m_Width;
		uint32_t m_Height;
	};
}
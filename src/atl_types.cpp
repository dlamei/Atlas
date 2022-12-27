#include "atl_types.h"

#include "gl_utils.h"
#include "gl_atl_utils.h"

void ImGui::Image(Atlas::Texture2D &texture, const ImVec2 &size, const ImVec2 &uv0, const ImVec2 &uv1,
	const ImVec4 &tint_col, const ImVec4 &border_col) {

	ImGui::Image((ImTextureID)texture.get_native(), size, uv0, uv1, tint_col, border_col);
}

namespace Atlas {

	uint32_t to_rgb(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
		uint32_t result = (a << 24) | (r << 16) | (g << 8) | b;
		return result;
	}

	Color::Color()
		: m_Data(to_rgb(255, 255, 255, 255)) {};
	Color::Color(uint8_t value)
		: m_Data(to_rgb(value, value, value, 255)) {};
	Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
		: m_Data(to_rgb(r, g, b, a)) {};
	Color::Color(uint8_t r, uint8_t g, uint8_t b)
		: m_Data(to_rgb(r, g, b, 255)) {};

	inline uint8_t Color::red() const
	{
		return m_Data >> 16 & 0xff;
	}

	inline uint8_t Color::green() const
	{
		return m_Data >> 8 & 0xff;
	}

	inline uint8_t Color::blue() const
	{
		return m_Data & 0xff;
	}

	inline uint8_t Color::alpha() const
	{
		return m_Data >> 24 & 0xff;
	}

	glm::vec4 Color::normalized_vec()
	{
		float r = red() / 255.f;
		float g = green() / 255.f;
		float b = blue() / 255.f;
		float a = alpha() / 255.f;

		return { r, g, b, a };
	}

	Color::operator uint32_t() const
	{
		return m_Data;
	}

	std::ostream &operator<<(std::ostream &os, const Color &c) {
		os << "{ r: " << (uint32_t)c.red() << ", g: " << (uint32_t)c.green() << ", b: "
			<< (uint32_t)c.blue() << ", a: " << (uint32_t)c.alpha() << " }";
		return os;
	}


	Texture2D::Texture2D(const Texture2DCreateInfo &info)
		: m_Format(info.format)
	{
		gl_utils::GLTexture2DCreateInfo texInfo{};
		texInfo.width = info.width;
		texInfo.height = info.height;
		texInfo.format = color_format_to_gl_enum(info.format);
		texInfo.mipmap = info.mipmap;
		texInfo.minFilter = texture_min_filter_to_gl_enum(info.filter, info.mipmap);
		texInfo.magFilter = texture_mag_filter_to_gl_enum(info.filter, info.mipmap);

		m_Texture = make_ref<gl_utils::GLTexture2D>(texInfo);
	}

	Texture2D Texture2D::color(uint32_t width, uint32_t height, TextureFilter filter)
	{
		Texture2DCreateInfo info{};
		info.width = width;
		info.height = height;
		info.mipmap = false;
		info.filter = filter;
		info.format = ColorFormat::R8G8B8A8;
		return Texture2D(info);
	}

	void Texture2D::set_data(Color *data)
	{
		CORE_ASSERT(m_Texture, "Texture2D::set_data: texture was not initialized!");
		m_Texture->set_data(data, color_format_to_gl_enum(m_Format));
	}

	void Texture2D::bind(uint32_t indx)
	{
		m_Texture->bind(indx);
	}

	uint32_t Texture2D::width()
	{
		CORE_ASSERT(m_Texture, "Texture2D::width: texture was not initialized!");
		return m_Texture->width();
	}

	uint32_t Texture2D::height()
	{
		CORE_ASSERT(m_Texture, "Texture2D::height: texture was not initialized!");
		return m_Texture->height();
	}

	bool Texture2D::has_mipmap()
	{
		CORE_ASSERT(m_Texture, "Texture2D::has_mipmap: texture was not initialized!");
		return m_Texture->has_mipmap();
	}

	uint32_t Texture2D::get_native()
	{
		return m_Texture->id();
	}

	FrameBuffer::FrameBuffer(std::vector<Attachment> attachments)
	{
		m_Framebuffer = make_ref<gl_utils::GLFramebuffer>();

		if (attachments.size() == 0) return;

		m_Width = attachments.at(0).width();
		m_Height = attachments.at(0).height();

		uint32_t colorAttachmentIndx{ 0 };

		for (Attachment a : attachments) {
			if (!a.is_init()) {
				CORE_WARN("FrameBuffer::FrameBuffer: texture is not initialized!");
				return;
			}

			if (!is_color_attachment(a.format())) {
				CORE_WARN("FrameBuffer::FrameBuffer: expected color format, found: {}", (uint32_t)a.format());
				continue;
			}

			auto attachment = color_format_to_gl_attachment(a.format());

			m_Framebuffer->push_tex_attachment(attachment + colorAttachmentIndx, a.get_native());
			m_ColorTextures.insert({ colorAttachmentIndx, a });
			colorAttachmentIndx++;
		}

		if (!m_Framebuffer->check_status()) {
			CORE_WARN("FrameBuffer::FrameBuffer: Framebuffer is not complete! error: 0x{:x}", m_Framebuffer->get_status());
		}
	}

	void FrameBuffer::set_color_attachment(Texture2D &texture, uint32_t index)
	{
		CORE_ASSERT(m_Framebuffer, "FrameBuffer::set_color_attachment: framebuffer was not initialized");
		if (!is_color_attachment(texture.format())) {
			CORE_WARN("FrameBuffer::set_color_attachment: texture has no color format");
			return;
		}

		m_ColorTextures.insert_or_assign(index, texture);
		m_Framebuffer->push_tex_attachment(color_format_to_gl_attachment(texture.format()) + index, texture.get_native());
	}

	void FrameBuffer::set_depth_stencil_texture(Texture2D &texture)
	{
		CORE_ASSERT(m_Framebuffer, "FrameBuffer::set_depth_stencil_texture: framebuffer was not initialized");
		if (is_color_attachment(texture.format())) {
			CORE_WARN("FrameBuffer::set_depth_stencil_texture: texture has color format");
			return;
		}

		m_DepthStencilTexture = texture;
		m_Framebuffer->push_tex_attachment(color_format_to_gl_attachment(texture.format()), texture.get_native());
	}

	void FrameBuffer::bind()
	{
		CORE_ASSERT(m_Framebuffer, "FrameBuffer::bind: framebuffer was not initialized");
		m_Framebuffer->bind();
	}

	void FrameBuffer::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

}

#include "atl_types.h"

#include "gl_utils.h"
#include "gl_atl_utils.h"

#include <stb_image.h>

void ImGui::Image(const Atlas::Texture2D &texture, const ImVec2 &size, const ImVec2 &uv0, const ImVec2 &uv1,
	const ImVec4 &tint_col, const ImVec4 &border_col) {
	uintptr_t ptr = texture.get_native();
	ImGui::Image(reinterpret_cast<void *>(ptr), size, uv0, uv1, tint_col, border_col);
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

	std::pair<uint32_t, uint32_t> vertex_attrib_to_gl_enum(VertexAttribute a)
	{
		switch (a)
		{
		case Atlas::VertexAttribute::INT:		return { GL_INT, 1 };
		case Atlas::VertexAttribute::INT2:		return { GL_INT, 2 };
		case Atlas::VertexAttribute::INT3:		return { GL_INT, 3 };
		case Atlas::VertexAttribute::INT4:		return { GL_INT, 4 };
		case Atlas::VertexAttribute::UINT:		return { GL_UNSIGNED_INT, 1 };
		case Atlas::VertexAttribute::UINT2:		return { GL_UNSIGNED_INT, 2 };
		case Atlas::VertexAttribute::UINT3:		return { GL_UNSIGNED_INT, 3 };
		case Atlas::VertexAttribute::UINT4:		return { GL_UNSIGNED_INT, 4 };
		case Atlas::VertexAttribute::FLOAT:		return { GL_FLOAT, 1 };
		case Atlas::VertexAttribute::FLOAT2:	return { GL_FLOAT, 2 };
		case Atlas::VertexAttribute::FLOAT3:	return { GL_FLOAT, 3 };
		case Atlas::VertexAttribute::FLOAT4:	return { GL_FLOAT, 4 };
		}

		CORE_ASSERT(false, "vertex_attrib_to_gl_enum: enum not defined!");
		return { 0, 0 };
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

	std::optional<Texture2D> Texture2D::load(const char *file, TextureFilter filter)
	{
		int width, height, channels;

		stbi_set_flip_vertically_on_load(true);

		stbi_uc *data = nullptr;
		data = stbi_load(file, &width, &height, &channels, 0);

		if (!data || stbi_failure_reason()) {
			CORE_WARN("Failed to load image: {}", file);
			CORE_WARN("{}", stbi_failure_reason());
			return std::nullopt;
		}

		Texture2DCreateInfo info{};
		info.width = width;
		info.height = height;
		info.mipmap = true;
		info.filter = filter;

		if (channels == 4)
		{
			info.format = ColorFormat::R8G8B8A8;
		}
		else if (channels == 3)
		{
			info.format = ColorFormat::R8G8B8;
		}

		Texture2D tex = Texture2D(info);
		tex.set_data((Color *)data);

		stbi_image_free(data);

		return tex;
	}

	void Texture2D::set_data(Color *data)
	{
		CORE_ASSERT(m_Texture, "Texture2D::set_data: texture was not initialized!");
		m_Texture->set_data(data, color_format_to_int_gl_enum(m_Format));
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

	uint32_t Texture2D::get_native() const
	{
		return m_Texture->id();
	}

	Framebuffer::Framebuffer(std::vector<Attachment> attachments)
	{
		m_Framebuffer = make_ref<gl_utils::GLFramebuffer>();

		if (attachments.size() == 0) return;

		m_Width = attachments.at(0).width();
		m_Height = attachments.at(0).height();

		uint32_t colorAttachmentIndx{ 0 };

		for (Attachment &a : attachments) {
			if (!a.is_init()) {
				CORE_WARN("Framebuffer::Framebuffer: texture is not initialized!");
				return;
			}

			if (!is_color_attachment(a.format())) {
				CORE_WARN("Framebuffer::Framebuffer: expected color format, found: {}", (uint32_t)a.format());
				continue;
			}

			auto attachment = color_format_to_gl_attachment(a.format());

			m_Framebuffer->push_tex_attachment(attachment + colorAttachmentIndx, a.get_native());
			m_ColorTextures.insert({ colorAttachmentIndx, a });
			colorAttachmentIndx++;
		}

		if (!m_Framebuffer->check_status()) {
			CORE_WARN("Framebuffer::Framebuffer: Framebuffer is not complete! error: 0x{:x}", m_Framebuffer->get_status());
		}
	}

	void Framebuffer::set_color_attachment(Texture2D &texture, uint32_t index)
	{
		CORE_ASSERT(m_Framebuffer, "Framebuffer::set_color_attachment: framebuffer was not initialized");
		if (!is_color_attachment(texture.format())) {
			CORE_WARN("Framebuffer::set_color_attachment: texture has no color format");
			return;
		}

		m_ColorTextures.insert_or_assign(index, texture);
		m_Framebuffer->push_tex_attachment(color_format_to_gl_attachment(texture.format()) + index, texture.get_native());
	}

	void Framebuffer::set_depth_stencil_texture(Texture2D &texture)
	{
		CORE_ASSERT(m_Framebuffer, "Framebuffer::set_depth_stencil_texture: framebuffer was not initialized");
		if (is_color_attachment(texture.format())) {
			CORE_WARN("Framebuffer::set_depth_stencil_texture: texture has color format");
			return;
		}

		m_DepthStencilTexture = texture;
		m_Framebuffer->push_tex_attachment(color_format_to_gl_attachment(texture.format()), texture.get_native());
	}

	const Texture2D &Framebuffer::get_color_attachment(uint32_t index)
	{
		CORE_ASSERT(index < m_ColorTextures.size(), "Framebuffer::get_color_attachment: error, index out of range!");
		return m_ColorTextures.at(index);
	}

	void Framebuffer::bind()
	{
		CORE_ASSERT(m_Framebuffer, "Framebuffer::bind: framebuffer was not initialized");
		m_Framebuffer->bind();
	}

	void Framebuffer::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	Framebuffer Framebuffer::empty()
	{
		std::vector<Texture2D> att;
		return Framebuffer(att);
	}

	Buffer::Buffer(const BufferCreateInfo &info)
		: m_Stride(info.stride), m_Types(info.types)
	{

		gl_utils::GLBufferCreateInfo buffInfo{};
		buffInfo.size = info.size;
		buffInfo.data = { info.data, info.size };
		buffInfo.usage = buffer_usage_to_gl_enum(info.usage);

		m_Buffer = make_ref<gl_utils::GLBuffer>(buffInfo);
	}

	//void Buffer::bind(BufferTypes type)
	//{
	//	CORE_ASSERT(m_Buffer, "Buffer::bind: buffer was not initialized!");
	//	CORE_ASSERT(type | m_Types, "Buffer::bind: this type was not set when initializing buffer!");

	//	if (type | BufferType::VERTEX) {
	//		gl_utils::bind_vertex_buffer(m_Buffer, m_Stride, m_Index, 0);
	//	}
	//	if (type | BufferType::INDEX_U32) {
	//		gl_utils::bind_index_buffer(m_Buffer);
	//	}
	//}

	void Buffer::bind_vertex(const Buffer &buffer, uint32_t index)
	{
		CORE_ASSERT(buffer.is_init(), "bind_vertex_buffer: buffer was not initialized!");

		if (buffer.type() | BufferType::VERTEX) {
			CORE_WARN("bind_vertex_buffer: buffer was not initialized as a vertex buffer");
			return;
		}

		gl_utils::bind_vertex_buffer(buffer.m_Buffer, buffer.m_Stride, index, 0);
	}

	void Buffer::bind_index(const Buffer &buffer)
	{
		CORE_ASSERT(buffer.is_init(), "bind_vertex_buffer: buffer was not initialized!");

		if (buffer.type() | BufferType::INDEX_U32) {
			CORE_WARN("bind_vertex_buffer: buffer was not initialized as a vertex buffer");
			return;
		}

		gl_utils::bind_index_buffer(buffer.m_Buffer);
	}

	void Buffer::set_data(void *data, size_t size) {
		CORE_ASSERT(m_Buffer, "Buffer::bind: buffer was not initialized!");
		CORE_ASSERT(size <= m_Buffer->size(), "Buffer::set_data: size has to be smaller or equal then {}. it is {}", m_Buffer->size(), size);
		m_Buffer->set_data(data, size);
	}

	size_t Buffer::size()
	{
		CORE_ASSERT(m_Buffer, "Buffer::bind: buffer was not initialized!");
		return m_Buffer->size();
	}

	VertexLayout VertexLayout::empty()
	{
		auto layout = VertexLayout();
		layout.m_Layout = make_ref<gl_utils::GLVertexLayout>();
		return layout;
	}

	void VertexLayout::push(uint32_t count, uint32_t gl_type, uint32_t offset)
	{
		CORE_ASSERT(m_Layout, "VertexLayout::push: buffer was not initialized!");
		m_Layout->push_attrib(count, gl_type, offset, m_BufferIndx);
	}

	void VertexLayout::set_index(uint32_t index)
	{
		CORE_ASSERT(m_Layout, "VertexLayout::set_index: buffer was not initialized!");
		m_BufferIndx = index;
	}

	void VertexLayout::bind() const
	{
		CORE_ASSERT(m_Layout, "VertexLayout::bind: buffer was not initialized!");
		m_Layout->bind();
	}

	Shader::Shader(const ShaderCreateInfo &info)
		: m_Layout(info.layout)
	{
		gl_utils::GLShaderCreateInfo shaderInfo{};

		for (auto &module : info.modules) {
			shaderInfo.push_back({ module.first, shader_type_to_gl_enum(module.second) });
		}

		m_Shader = make_ref<gl_utils::GLShader>(shaderInfo);
	}

	void Shader::bind(const Shader &shader)
	{
		CORE_ASSERT(shader.is_init(), "Shader::bind: Shader was not initialized!");

		shader.m_Layout.bind();
		gl_utils::bind_shader(shader.m_Shader);
	}

	void Shader::unbind()
	{
		glUseProgram(0);
	}

	Shader Shader::load(const char *vertexFile, const char *fragFile, const VertexLayout &layout)
	{
		ShaderCreateInfo info{};
		info.layout = layout;
		info.modules.push_back({ vertexFile, ShaderType::VERTEX });
		info.modules.push_back({ fragFile, ShaderType::FRAGMENT });
		return Shader(info);
	}

	void Shader::set_int(const char *name, int32_t value)
	{
		CORE_ASSERT(m_Shader, "Shader::set_int: Shader was not initialized!");
		m_Shader->set_int(name, value);
	}

	void Shader::set_int2(const char *name, const glm::ivec2 &value)
	{
		CORE_ASSERT(m_Shader, "Shader::set_int2: Shader was not initialized!");
		m_Shader->set_int2(name, value);
	}

	void Shader::set_int3(const char *name, const glm::ivec3 &value)
	{
		CORE_ASSERT(m_Shader, "Shader::set_int3: Shader was not initialized!");
		m_Shader->set_int3(name, value);
	}

	void Shader::set_int4(const char *name, const glm::ivec4 &value)
	{
		CORE_ASSERT(m_Shader, "Shader::set_int4: Shader was not initialized!");
		m_Shader->set_int4(name, value);
	}

	void Shader::set_int_vec(const char *name, int32_t *data, size_t count)
	{
		CORE_ASSERT(m_Shader, "Shader::set_int_vec: Shader was not initialized!");
		m_Shader->set_int_vec(name, data, count);
	}

	void Shader::set_uint(const char *name, uint32_t value)
	{
		CORE_ASSERT(m_Shader, "Shader::set_uint: Shader was not initialized!");
		m_Shader->set_uint(name, value);
	}

	void Shader::set_uint2(const char *name, const glm::uvec2 &value)
	{
		CORE_ASSERT(m_Shader, "Shader::set_uint2: Shader was not initialized!");
		m_Shader->set_uint2(name, value);
	}

	void Shader::set_uint3(const char *name, const glm::uvec3 &value)
	{
		CORE_ASSERT(m_Shader, "Shader::set_uint3: Shader was not initialized!");
		m_Shader->set_uint3(name, value);
	}

	void Shader::set_uint4(const char *name, const glm::uvec4 &value)
	{
		CORE_ASSERT(m_Shader, "Shader::set_uint4: Shader was not initialized!");
		m_Shader->set_uint4(name, value);
	}

	void Shader::set_uint_vec(const char *name, uint32_t *data, size_t count)
	{
		CORE_ASSERT(m_Shader, "Shader::set_uint_vec: Shader was not initialized!");
		m_Shader->set_uint_vec(name, data, count);
	}

	void Shader::set_float(const char *name, float value)
	{
		CORE_ASSERT(m_Shader, "Shader::set_float: Shader was not initialized!");
		m_Shader->set_float(name, value);
	}

	void Shader::set_float2(const char *name, const glm::vec2 &value)
	{
		CORE_ASSERT(m_Shader, "Shader::set_float2: Shader was not initialized!");
		m_Shader->set_float2(name, value);
	}

	void Shader::set_float3(const char *name, const glm::vec3 &value)
	{
		CORE_ASSERT(m_Shader, "Shader::set_float3: Shader was not initialized!");
		m_Shader->set_float3(name, value);
	}

	void Shader::set_float4(const char *name, const glm::vec4 &value)
	{
		CORE_ASSERT(m_Shader, "Shader::set_float4: Shader was not initialized!");
		m_Shader->set_float4(name, value);
	}

	void Shader::set_float_vec(const char *name, float *data, size_t count)
	{
		CORE_ASSERT(m_Shader, "Shader::set_float_vec: Shader was not initialized!");
		m_Shader->set_float_vec(name, data, count);
	}

	void Shader::set_mat3(const char *name, const glm::mat3 &value)
	{
		CORE_ASSERT(m_Shader, "Shader::set_mat3: Shader was not initialized!");
		m_Shader->set_mat3(name, value);
	}

	void Shader::set_mat4(const char *name, const glm::mat4 &value)
	{
		CORE_ASSERT(m_Shader, "Shader::set_mat4: Shader was not initialized!");
		m_Shader->set_mat4(name, value);
	}
}


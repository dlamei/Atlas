#include "atl_types.h"

#include "gl_utils.h"
#include "gl_atl_utils.h"

#include <stb_image.h>

void ImGui::Image(const Atlas::Texture2D &texture, const ImVec2 &size, const ImVec2 &uv0, const ImVec2 &uv1,
	const ImVec4 &tint_col, const ImVec4 &border_col) {
	uintptr_t ptr = texture.m_Texture->id();
	ImGui::Image(reinterpret_cast<void *>(ptr), size, uv0, uv1, tint_col, border_col);
}

namespace Atlas {

	namespace RenderApi {

		struct CachedFramebuffer {
			Framebuffer framebuffer;
			bool used{ false };
		};

		struct RenderContext {
			std::unordered_map<size_t, CachedFramebuffer> framebuffers;
			bool clearColorBuffer{ true };
			bool clearDepthBuffer{ true };
			glm::vec4 clearColor{ 0, 0, 0, 0 };
		};

		static RenderContext GlobalRenderContext{};

		size_t framebuffer_create_hash(const FramebufferCreateInfo &info) {
			size_t hash = 0;

			for (const auto &col : info.colorAttachments) {
				if (col.is_init()) {
					hash ^= col.hash();
				}
			}

			if (info.depthAttachments.is_init()) hash ^= info.depthAttachments.hash();

			return hash;
		}

		Framebuffer get_framebuffer(const FramebufferCreateInfo &info) {

			size_t hash = framebuffer_create_hash(info);

			if (GlobalRenderContext.framebuffers.find(hash) != GlobalRenderContext.framebuffers.end()) {
				auto it = GlobalRenderContext.framebuffers.find(hash);
				it->second.used = true;
				return it->second.framebuffer;
			}

			Framebuffer fb{ info };
			CachedFramebuffer cFb{};
			cFb.framebuffer = fb;
			cFb.used = true;

			GlobalRenderContext.framebuffers.insert({ hash, cFb });

			return fb;
		}

		void begin(const Texture2D &color) {

			FramebufferCreateInfo fbInfo{};
			fbInfo.depthAttachments = { color };

			Framebuffer fb = get_framebuffer(fbInfo);
			begin(fb);
		}

		void begin(const Texture2D &color, const Texture2D &depth)
		{
			FramebufferCreateInfo fbInfo{};
			fbInfo.colorAttachments = { color };
			fbInfo.depthAttachments = depth;

			Framebuffer fb = get_framebuffer(fbInfo);
			begin(fb);
		}

		void begin(const Framebuffer &frameBuffer)
		{
			glm::vec4 col = GlobalRenderContext.clearColor;

			Framebuffer::bind(frameBuffer);

			glClearColor(col.r, col.g, col.b, col.a);
			glClear(GlobalRenderContext.clearColorBuffer ? GL_COLOR_BUFFER_BIT : 0 || GlobalRenderContext.clearDepthBuffer ? GL_DEPTH_BUFFER_BIT : 0);
		}

		void frame_start()
		{
			for (auto &it : GlobalRenderContext.framebuffers) {
				it.second.used = false;
			}
		}

		void frame_end()
		{
			auto &framebuffers = GlobalRenderContext.framebuffers;

			//TODO: maybe only delete when not used for multiple frames
			for (auto it = framebuffers.begin(); it != framebuffers.end();) {
				if (!it->second.used) it = framebuffers.erase(it);
				else it++;
			}

		}

		void enable_clear_color(bool b)
		{
			GlobalRenderContext.clearColorBuffer = b;
		}

		void enable_clear_depth(bool b)
		{
			GlobalRenderContext.clearDepthBuffer = b;
		}

		void clear_color(Color c)
		{
			GlobalRenderContext.clearColor = c.normalized();
		}

		void end()
		{
			Framebuffer::unbind();
		}

		void draw_indexed(size_t indexCount)
		{
			glDrawElements(GL_TRIANGLES, (int)indexCount, GL_UNSIGNED_INT, 0);
		}

		void init()
		{
			gl_utils::init_opengl();
		}

		void resize_viewport(uint32_t width, uint32_t height)
		{
			gl_utils::resize_viewport(width, height);
		}
	}

	struct BindingContext {
		Shader shader;
		std::unordered_map<uint32_t, Texture2D> textures;
		Framebuffer framebuffer;
		Buffer indexBuffer;
		Buffer storageBuffer;
		std::unordered_map<uint32_t, Buffer> vertexBuffers;
		VertexLayout layout;
	};

	static BindingContext GlobalBindingContext{};

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

	glm::vec4 Color::normalized()
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

	Texture2D Texture2D::depth(uint32_t width, uint32_t height, TextureFilter filter)
	{
		Texture2DCreateInfo info{};
		info.width = width;
		info.height = height;
		info.mipmap = false;
		info.filter = filter;
		info.format = ColorFormat::D32;
		return Texture2D(info);
	}

	Texture2D Texture2D::depth_stencil(uint32_t width, uint32_t height, TextureFilter filter)
	{
		Texture2DCreateInfo info{};
		info.width = width;
		info.height = height;
		info.mipmap = false;
		info.filter = filter;
		info.format = ColorFormat::D24S8;
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

	void Texture2D::bind(const Texture2D &texture, uint32_t indx)
	{
		CORE_ASSERT(texture.is_init(), "Texture2D::bind: texture was not initialized!");

		if (GlobalBindingContext.textures.find(indx) != GlobalBindingContext.textures.end()) return;

		texture.m_Texture->bind(indx);

		GlobalBindingContext.textures.insert_or_assign(indx, texture);
	}

	void Texture2D::set_data(Color *data) const
	{
		CORE_ASSERT(m_Texture, "Texture2D::set_data: texture was not initialized!");
		m_Texture->set_data(data, color_format_to_int_gl_enum(m_Format));
	}

	//void Texture2D::bind(uint32_t indx) const
	//{
	//	m_Texture->bind(indx);
	//}

	uint32_t Texture2D::width() const
	{
		CORE_ASSERT(m_Texture, "Texture2D::width: texture was not initialized!");
		return m_Texture->width();
	}

	uint32_t Texture2D::height() const
	{
		CORE_ASSERT(m_Texture, "Texture2D::height: texture was not initialized!");
		return m_Texture->height();
	}

	bool Texture2D::has_mipmap() const
	{
		CORE_ASSERT(m_Texture, "Texture2D::has_mipmap: texture was not initialized!");
		return m_Texture->has_mipmap();
	}

	size_t Texture2D::hash() const
	{
		return std::hash<void *>()(m_Texture.get());
	}

	Framebuffer::Framebuffer(const FramebufferCreateInfo &info)
	{
		m_Framebuffer = make_ref<gl_utils::GLFramebuffer>();

		if (info.colorAttachments.size() != 0) {
			CORE_ASSERT(info.colorAttachments.at(0).is_init(), "Framebuffer::Framebuffer: first colorattachment is not initialized");
			m_Width = info.colorAttachments.at(0).width();
			m_Height = info.colorAttachments.at(0).height();
		}
		else if (info.depthAttachments.is_init()) {
			m_Width = info.depthAttachments.width();
			m_Height = info.depthAttachments.height();
		}
		else {
			return;
		}


		uint32_t colorAttachmentIndx{ 0 };

		for (const FramebufferAttachment &a : info.colorAttachments) {
			if (!a.is_init()) {
				CORE_WARN("Framebuffer::Framebuffer: texture is not initialized!");
				return;
			}

			if (!is_color_attachment(a.format())) {
				CORE_WARN("Framebuffer::Framebuffer: expected color format, found: {}", (uint32_t)a.format());
				continue;
			}

			auto attachment = color_format_to_gl_attachment(a.format());

			m_Framebuffer->push_tex_attachment(attachment + colorAttachmentIndx, a.m_Texture->id());
			m_ColorTextures.insert({ colorAttachmentIndx, a });
			colorAttachmentIndx++;
		}

		if (info.depthAttachments.is_init()) {
			if (is_color_attachment(info.depthAttachments.format())) {
				CORE_WARN("Framebuffer::Framebuffer: expected depth format, found: {}", (uint32_t)info.depthAttachments.format());
			}

			auto attachment = color_format_to_gl_attachment(info.depthAttachments.format());
			m_Framebuffer->push_tex_attachment(attachment, info.depthAttachments.m_Texture->id());
			m_DepthStencilTexture = info.depthAttachments;
		}

		if (!m_Framebuffer->check_status()) {
			CORE_WARN("Framebuffer::Framebuffer: Framebuffer is not complete! error: 0x{:x}", m_Framebuffer->get_status());
		}
	}

	void Framebuffer::set_color_attachment(const Texture2D &texture, uint32_t index)
	{
		CORE_ASSERT(m_Framebuffer, "Framebuffer::set_color_attachment: framebuffer was not initialized");
		if (!is_color_attachment(texture.format())) {
			CORE_WARN("Framebuffer::set_color_attachment: texture has no color format");
			return;
		}

		m_ColorTextures.insert_or_assign(index, texture);
		m_Framebuffer->push_tex_attachment(color_format_to_gl_attachment(texture.format()) + index, texture.m_Texture->id());
	}

	void Framebuffer::set_depth_stencil_texture(const Texture2D &texture)
	{
		CORE_ASSERT(m_Framebuffer, "Framebuffer::set_depth_stencil_texture: framebuffer was not initialized");
		if (is_color_attachment(texture.format())) {
			CORE_WARN("Framebuffer::set_depth_stencil_texture: texture has color format");
			return;
		}

		m_DepthStencilTexture = texture;
		m_Framebuffer->push_tex_attachment(color_format_to_gl_attachment(texture.format()), texture.m_Texture->id());
	}

	const Texture2D &Framebuffer::get_color_attachment(uint32_t index)
	{
		CORE_ASSERT(index < m_ColorTextures.size(), "Framebuffer::get_color_attachment: error, index out of range!");
		return m_ColorTextures.at(index);
	}

	size_t Framebuffer::hash() const
	{
		return std::hash<void *>()(m_Framebuffer.get());
	}

	void Framebuffer::bind(const Framebuffer &framebuffer)
	{
		CORE_ASSERT(framebuffer.is_init(), "Framebuffer::bind: framebuffer was not initialized");
		if (GlobalBindingContext.framebuffer == framebuffer) return;

		framebuffer.m_Framebuffer->bind();
		GlobalBindingContext.framebuffer = framebuffer;
	}

	void Framebuffer::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GlobalBindingContext.framebuffer = Framebuffer();
	}

	Framebuffer Framebuffer::empty()
	{
		FramebufferCreateInfo info{};
		return Framebuffer(info);
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

	void Buffer::bind_vertex(const Buffer &buffer, uint32_t index)
	{
		CORE_ASSERT(buffer.is_init(), "bind_vertex_buffer: buffer was not initialized!");

		if (!(buffer.type() | BufferType::VERTEX)) {
			CORE_WARN("Buffer::bind_vertex: buffer was not initialized as a vertex buffer");
			return;
		}

		if (GlobalBindingContext.vertexBuffers.find(index) != GlobalBindingContext.vertexBuffers.end()) return;

		gl_utils::bind_vertex_buffer(buffer.m_Buffer, buffer.m_Stride, index, 0);

		GlobalBindingContext.vertexBuffers.insert_or_assign(index, buffer);
	}

	void Buffer::bind_index(const Buffer &buffer)
	{
		CORE_ASSERT(buffer.is_init(), "bind_vertex_buffer: buffer was not initialized!");

		if (!(buffer.type() | BufferType::INDEX_U32)) {
			CORE_WARN("Buffer::bind_index: buffer was not initialized as a vertex buffer");
			return;
		}

		if (GlobalBindingContext.indexBuffer == buffer) return;

		gl_utils::bind_index_buffer(buffer.m_Buffer);

		GlobalBindingContext.indexBuffer = buffer;
	}

	void Buffer::bind_storage(const Buffer &buffer)
	{
		CORE_ASSERT(buffer.is_init(), "bind_vertex_buffer: buffer was not initialized!");

		if (!(buffer.type() | BufferType::STORAGE)) {
			CORE_WARN("Buffer::bind_storage: buffer was not initialized as a vertex buffer");
			return;
		}

		if (GlobalBindingContext.storageBuffer == buffer) return;

		gl_utils::bind_storage(buffer.m_Buffer);

		GlobalBindingContext.storageBuffer = buffer;
	}

	size_t Buffer::hash() const
	{
		return std::hash<void *>()(m_Buffer.get());
	}

	Buffer Buffer::create(BufferTypes types, void *data, size_t size, BufferUsage usage, size_t stride)
	{
		BufferCreateInfo info{};
		info.size = size;
		info.data = data;
		info.types = types;
		info.usage = usage;
		info.stride = stride ? stride : size;

		return Buffer(info);
	}

	Buffer Buffer::uniform(void *data, size_t size, BufferUsage usage)
	{
		BufferCreateInfo info{};
		info.size = size;
		info.data = data;
		info.types = BufferType::UNIFORM;
		info.usage = usage;
		info.stride = size;

		return Buffer(info);
	}

	Buffer Buffer::storage(void *data, size_t size, BufferUsage usage)
	{
		BufferCreateInfo info{};
		info.size = size;
		info.data = data;
		info.types = BufferType::STORAGE;
		info.usage = usage;
		info.stride = size;

		return Buffer(info);
	}

	Buffer Buffer::index(uint32_t *data, size_t count, BufferUsage usage) {
		BufferCreateInfo info{};
		info.size = sizeof(uint32_t) * count;
		info.data = (void *)data;
		info.types = BufferType::INDEX_U32;
		info.usage = usage;
		info.stride = sizeof(uint32_t);

		return Buffer(info);
	}

	void Buffer::set_data(void *data, size_t size) {
		CORE_ASSERT(m_Buffer, "Buffer::bind: buffer was not initialized!");
		CORE_ASSERT(size <= m_Buffer->size(), "Buffer::set_data: size has to be smaller or equal then {}. it is {}", m_Buffer->size(), size);
		m_Buffer->set_data(data, size);
	}

	size_t Buffer::size() const
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

	//void VertexLayout::push(uint32_t count, uint32_t gl_type, uint32_t offset)
	//{
	//	CORE_ASSERT(m_Layout, "VertexLayout::push: buffer was not initialized!");
	//	m_Layout->push_attrib(count, gl_type, offset, m_BufferIndx);
	//}

	void VertexLayout::push(VertexAttribute attribute, uint32_t offset)
	{
		CORE_ASSERT(m_Layout, "VertexLayout::push: buffer was not initialized!");
		auto [type, count] = vertex_attrib_to_gl_enum(attribute);
		m_Layout->push_attrib(count, type, offset, m_BufferIndx);
	}

	void VertexLayout::set_index(uint32_t index)
	{
		CORE_ASSERT(m_Layout, "VertexLayout::set_index: buffer was not initialized!");
		m_BufferIndx = index;
	}

	void VertexLayout::bind(const VertexLayout &layout)
	{
		CORE_ASSERT(layout.is_init(), "VertexLayout::bind: buffer was not initialized!");

		if (GlobalBindingContext.layout == layout) return;

		layout.m_Layout->bind();

		GlobalBindingContext.layout = layout;
	}

	size_t VertexLayout::hash() const
	{
		return std::hash<void *>()(m_Layout.get());
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

		if (GlobalBindingContext.shader == shader) return;

		for (auto &pair : shader.m_UniformBuffers) {
			const Buffer &buff = pair.second;
			gl_utils::bind_uniform_buffer(shader.m_Shader->get_block_binding(pair.first), pair.second.m_Buffer);
		}

		VertexLayout::bind(shader.m_Layout);
		gl_utils::bind_shader(shader.m_Shader);

		GlobalBindingContext.shader = shader;
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

#define IMPL_SHADER_FUNC(TYPE, FUNC_NAME) \
	void Shader::set(const char *name, TYPE value) \
	{ \
		CORE_ASSERT(m_Shader, "Shader::set_int: Shader was not initialized!"); \
		m_Shader->FUNC_NAME(name, value); \
	}

	IMPL_SHADER_FUNC(int32_t, set_int);
	IMPL_SHADER_FUNC(const glm::ivec2 &, set_int2);
	IMPL_SHADER_FUNC(const glm::ivec3 &, set_int3);
	IMPL_SHADER_FUNC(const glm::ivec4 &, set_int4);

	IMPL_SHADER_FUNC(uint32_t, set_int);
	IMPL_SHADER_FUNC(const glm::uvec2 &, set_uint2);
	IMPL_SHADER_FUNC(const glm::uvec3 &, set_uint3);
	IMPL_SHADER_FUNC(const glm::uvec4 &, set_uint4);

	IMPL_SHADER_FUNC(float, set_float);
	IMPL_SHADER_FUNC(const glm::vec2 &, set_float2);
	IMPL_SHADER_FUNC(const glm::vec3 &, set_float3);
	IMPL_SHADER_FUNC(const glm::vec4 &, set_float4);

	IMPL_SHADER_FUNC(const glm::mat3 &, set_mat3);
	IMPL_SHADER_FUNC(const glm::mat4 &, set_mat4);

	void Shader::set(const char *name, const Buffer &buffer)
	{
		CORE_ASSERT(buffer.m_Types | BufferType::UNIFORM, "Shader::set: buffer was not initialized as unifrom buffer!");
		m_UniformBuffers.insert_or_assign(name, buffer);
	}

	Buffer &Shader::get_uniform_buffer(const char *name)
	{
		auto it = m_UniformBuffers.find(name);
		CORE_ASSERT(it != m_UniformBuffers.end(), "Shader::get_uniform_buffer: could not find buffer: {}", name);

		return m_UniformBuffers.at(name);
	}

	size_t Shader::hash() const
	{
		return std::hash<void *>()(m_Shader.get());
	}

	bool operator==(const Texture2D &t1, const Texture2D &t2)
	{
		return t1.m_Texture == t2.m_Texture;
	}
	bool operator!=(const Texture2D &t1, const Texture2D &t2)
	{
		return !(t1 == t2);
	}
	bool operator==(const Framebuffer &f1, const Framebuffer &f2)
	{
		return f1.m_Framebuffer == f2.m_Framebuffer;
	}
	bool operator!=(const Framebuffer &f1, const Framebuffer &f2)
	{
		return !(f1 == f2);
	}
	bool operator==(const Buffer &b1, const Buffer &b2)
	{
		return b1.m_Buffer == b2.m_Buffer;
	}
	bool operator!=(const Buffer &b1, const Buffer &b2)
	{
		return !(b1 == b2);
	}
	bool operator==(const VertexLayout &v1, const VertexLayout &v2)
	{
		return v1.m_Layout == v2.m_Layout;
	}

	bool operator!=(const VertexLayout &v1, const VertexLayout &v2)
	{
		return !(v1 == v2);
	}

	bool operator==(const Shader &s1, const Shader &s2)
	{
		return s1.m_Shader == s2.m_Shader;
	}

	bool operator!=(const Shader &s1, const Shader &s2)
	{
		return !(s1 == s2);
	}
}

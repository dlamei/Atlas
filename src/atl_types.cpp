#include "atl_types.h"

#include "gl_utils.h"
#include "gl_atl_utils.h"

#include <stb_image.h>

void ImGui::Image(const Atlas::Texture2D &texture, const ImVec2 &size, const ImVec2 &uv0, const ImVec2 &uv1,
	const ImVec4 &tint_col, const ImVec4 &border_col) {
	uintptr_t ptr = texture.m_Texture->id();
	ImGui::Image(reinterpret_cast<void *>(ptr), size, uv0, uv1, tint_col, border_col);
}

bool ImGui::ImageButton(const Atlas::Texture2D &texture, const ImVec2 &size, const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col)
{
	uintptr_t ptr = texture.m_Texture->id();
	return ImGui::ImageButton(reinterpret_cast<void *>(ptr), size, uv0, uv1, frame_padding, bg_col, tint_col);
}

namespace Atlas::Random {

	static thread_local std::mt19937 s_RandomEngine;
	static std::uniform_int_distribution<int64_t> s_IntDistribution;
	static std::uniform_real_distribution<double> s_RealDistribution;

	void init()
	{
		s_RandomEngine.seed(std::random_device()());
	}

	int64_t uniform_integer()
	{
		return s_IntDistribution(s_RandomEngine);
	}

	double uniform_real()
	{
		return s_RealDistribution(s_RandomEngine);
	}
}

namespace Atlas {


	struct BindingContext {
		VertexLayout layout;
		Shader shader;
		Framebuffer framebuffer;
		Buffer indexBuffer;

		std::unordered_map<uint32_t, Texture2D> textures;
		std::unordered_map<uint32_t, Buffer> vertexBuffers;
	};

	static BindingContext s_GlobalBindingContext;

	inline uint32_t to_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
		return (a << 24) | (b << 16) | (g << 8) | r;
	}

	RGBA::RGBA()
		: m_Data(to_rgba(255, 255, 255, 255)) {};
	RGBA::RGBA(uint8_t value)
		: m_Data(to_rgba(value, value, value, 255)) {};
	RGBA::RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
		: m_Data(to_rgba(r, g, b, a)) {};
	RGBA::RGBA(uint8_t r, uint8_t g, uint8_t b)
		: m_Data(to_rgba(r, g, b, 255)) {}

	RGBA RGBA::from_norm(glm::vec3 val)
	{
		uint8_t r = static_cast<uint8_t>(val.r * 255);
		uint8_t g = static_cast<uint8_t>(val.g * 255);
		uint8_t b = static_cast<uint8_t>(val.b * 255);
		return RGBA(r, g, b);
	}

	RGBA RGBA::from_norm(glm::vec4 val)
	{
		uint8_t r = static_cast<uint8_t>(val.r * 255);
		uint8_t g = static_cast<uint8_t>(val.g * 255);
		uint8_t b = static_cast<uint8_t>(val.b * 255);
		uint8_t a = static_cast<uint8_t>(val.a * 255);
		return RGBA(r, g, b, a);
	}

	inline uint8_t RGBA::red() const
	{
		return m_Data & 0xff;
	}

	inline uint8_t RGBA::green() const
	{
		return m_Data >> 8 & 0xff;
	}

	inline uint8_t RGBA::blue() const
	{
		return m_Data >> 16 & 0xff;
	}

	inline uint8_t RGBA::alpha() const
	{
		return m_Data >> 24 & 0xff;
	}

	glm::vec4 RGBA::normalized()
	{
		float r = red() / 255.f;
		float g = green() / 255.f;
		float b = blue() / 255.f;
		float a = alpha() / 255.f;

		return { r, g, b, a };
	}

	RGBA::operator uint32_t() const
	{
		return m_Data;
	}

	RGBA::operator RGB() const
	{
		return RGB(red(), green(), blue());
	}

	std::ostream &operator<<(std::ostream &os, const RGBA &c) {
		os << "{ r: " << (uint32_t)c.red() << ", g: " << (uint32_t)c.green() << ", b: "
			<< (uint32_t)c.blue() << ", a: " << (uint32_t)c.alpha() << " }";
		return os;
	}

	RGB::RGB()
		: m_Red(255), m_Blue(255), m_Green(255) {}

	RGB::RGB(uint8_t value)
		: m_Red(value), m_Blue(value), m_Green(value) {}

	RGB::RGB(uint8_t r, uint8_t g, uint8_t b)
		: m_Red(r), m_Blue(g), m_Green(b) {}

	RGB RGB::from_norm(glm::vec3 val)
	{
		uint8_t r = static_cast<uint8_t>(val.r * 255);
		uint8_t g = static_cast<uint8_t>(val.g * 255);
		uint8_t b = static_cast<uint8_t>(val.b * 255);
		return RGB(r, g, b);
	}

	glm::vec3 RGB::normalized()
	{
		float r = red() / 255.f;
		float g = green() / 255.f;
		float b = blue() / 255.f;

		return { r, g, b };
	}

	RGB::operator RGBA() const
	{
		return RGBA(m_Red, m_Green, m_Blue);
	}

	std::ostream &operator<<(std::ostream &os, const RGB &c) {
		os << "{ r: " << (uint32_t)c.red() << ", g: " << (uint32_t)c.green() << ", b: "
			<< (uint32_t)c.blue() << " }";
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

	Texture2D Texture2D::rgba(uint32_t width, uint32_t height, TextureFilter filter)
	{
		Texture2DCreateInfo info{};
		info.width = width;
		info.height = height;
		info.mipmap = false;
		info.filter = filter;
		info.format = ColorFormat::R8G8B8A8;
		return Texture2D(info);
	}

	Texture2D Texture2D::rgb(uint32_t width, uint32_t height, TextureFilter filter)
	{
		Texture2DCreateInfo info{};
		info.width = width;
		info.height = height;
		info.mipmap = false;
		info.filter = filter;
		info.format = ColorFormat::R8G8B8;
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
			CORE_WARN("Failed to load_vert_frag image: {}", file);
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
		tex.fill((RGBA *)data, (size_t)width * height * channels);

		stbi_image_free(data);

		return tex;
	}

	void Texture2D::bind(const Texture2D &texture, uint32_t indx, TextureUsageBits usage)
	{
		CORE_ASSERT(texture.is_init(), "Texture2D::bind: texture was not initialized!");

		if (usage == TextureUsage::SAMPLER) {

			auto it = s_GlobalBindingContext.textures.find(indx);
			if (it != s_GlobalBindingContext.textures.end() && it->second == texture) return;
			s_GlobalBindingContext.textures.insert_or_assign(indx, texture);

			glActiveTexture(GL_TEXTURE0 + indx);
			glBindTexture(GL_TEXTURE_2D, texture.m_Texture->id());
		}
		else if (usage & (TextureUsage::READ | TextureUsage::WRITE)) {
			GLenum glUsage = 0;
			if (usage == TextureUsage::READ) glUsage = GL_READ_ONLY;
			if (usage == TextureUsage::WRITE) glUsage = GL_WRITE_ONLY;
			if (usage == (TextureUsage::WRITE | TextureUsage::READ)) glUsage = GL_READ_WRITE;

			glBindImageTexture(indx, texture.m_Texture->id(), 0, GL_FALSE, 0, glUsage, color_format_to_gl_enum(texture.m_Format));
		}
		else {
			CORE_WARN("Texture2D::bind: unknown texture usage: {}", usage);
		}
	}

	void Texture2D::unbind(uint32_t index)
	{
		s_GlobalBindingContext.textures.erase(index);
		glActiveTexture(GL_TEXTURE0 + index);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	//void Texture2D::bind_image(const Texture2D &texture, uint32_t unit)
	//{
	//	glBindImageTexture(unit, texture.m_Texture->id(), 0, GL_FALSE, 0, GL_READ_WRITE, color_format_to_gl_enum(texture.m_Format));
	//}

	void Texture2D::fill(const void *data, size_t size) const
	{
		CORE_ASSERT(m_Texture, "Texture2D::set_data: texture was not initialized!");
		uint32_t dataSize = width() * height() * color_format_to_bytes(m_Format);
		CORE_ASSERT(dataSize == size, "Texture2D::set_data: size == width * height");
		ATL_EVENT();
		m_Texture->set_data(data, color_format_to_int_gl_enum(m_Format));
	}

	//void Texture2D::bind(uint32_t indx) const
	//{
	//	m_Texture->bind(indx);
	//}

	inline uint32_t Texture2D::width() const
	{
		CORE_ASSERT(m_Texture, "Texture2D::width: texture was not initialized!");
		return m_Texture->width();
	}

	inline uint32_t Texture2D::height() const
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
				CORE_WARN("Framebuffer::Framebuffer: expected rgba format, found: {}", (uint32_t)a.format());
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
			CORE_WARN("Framebuffer::set_color_attachment: texture has no rgba format");
			return;
		}

		m_ColorTextures.insert_or_assign(index, texture);
		m_Framebuffer->push_tex_attachment(color_format_to_gl_attachment(texture.format()) + index, texture.m_Texture->id());
	}

	void Framebuffer::set_depth_stencil_texture(const Texture2D &texture)
	{
		CORE_ASSERT(m_Framebuffer, "Framebuffer::set_depth_stencil_texture: framebuffer was not initialized");
		if (is_color_attachment(texture.format())) {
			CORE_WARN("Framebuffer::set_depth_stencil_texture: texture has rgba format");
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
		if (s_GlobalBindingContext.framebuffer == framebuffer) return;
		s_GlobalBindingContext.framebuffer = framebuffer;
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.m_Framebuffer->id());
	}

	void Framebuffer::unbind()
	{
		s_GlobalBindingContext.framebuffer = Framebuffer();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
		buffInfo.data = info.data;
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

		auto it = s_GlobalBindingContext.vertexBuffers.find(index);
		if (it != s_GlobalBindingContext.vertexBuffers.end()) return;
		s_GlobalBindingContext.vertexBuffers.insert_or_assign(index, buffer);
		gl_utils::bind_vertex_buffer(buffer.m_Buffer, buffer.m_Stride, index);
	}

	void Buffer::unbind_vertex(uint32_t index)
	{
		s_GlobalBindingContext.vertexBuffers.erase(index);
		glBindVertexBuffer(index, 0, 0, 0);
	}

	void Buffer::bind_index(const Buffer &buffer)
	{
		CORE_ASSERT(buffer.is_init(), "bind_vertex_buffer: buffer was not initialized!");

		if (!(buffer.type() | BufferType::INDEX_U32)) {
			CORE_WARN("Buffer::bind_index: buffer was not initialized as a vertex buffer");
			return;
		}

		if (s_GlobalBindingContext.indexBuffer == buffer) return;

		s_GlobalBindingContext.indexBuffer = buffer;
		gl_utils::bind_index_buffer(buffer.m_Buffer);
	}

	void Buffer::unbind_index()
	{
		glBindBuffer(GL_INDEX_BUFFER, 0);
	}

	void Buffer::map_write(const Buffer &buffer, std::function<void(void *)> func)
	{
		//CORE_ASSERT(buffer.type() & (BufferType::UNIFORM | BufferType::VERTEX | BufferType::STORAGE | BufferType::INDEX_U32), "Buffer::map_write: unsuported buffer type");
		void *data = glMapNamedBuffer(buffer.m_Buffer->id(), GL_MAP_WRITE_BIT);
		func(data);
		glUnmapNamedBuffer(buffer.m_Buffer->id());
	}

	size_t Buffer::hash() const
	{
		return std::hash<void *>()(m_Buffer.get());
	}

	Buffer Buffer::create(BufferTypeBits types, void *data, size_t size, BufferUsage usage, size_t stride)
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

	Buffer Buffer::index(size_t count, BufferUsage usage)
	{
		BufferCreateInfo info{};
		info.size = sizeof(uint32_t) * count;
		info.data = nullptr;
		info.types = BufferType::INDEX_U32;
		info.usage = usage;
		info.stride = sizeof(uint32_t);

		return Buffer(info);
	}

	void Buffer::set_data(void *data, size_t size) {
		CORE_ASSERT(m_Buffer, "Buffer::bind: buffer was not initialized!");
		CORE_ASSERT(size <= m_Buffer->size(), "Buffer::set_data: size has to be smaller or equal then {}. it is {}", m_Buffer->size(), size);
		ATL_EVENT();
		m_Buffer->set_data(data, size);
	}

	std::vector<void *> Buffer::get_data()
	{
		std::vector<void *> buffer;
		buffer.resize(m_Buffer->size());

		glGetNamedBufferSubData(m_Buffer->id(), 0, m_Buffer->size(), buffer.data());

		return buffer;
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

	void VertexLayout::push(VertexAttribute attribute, uint32_t offset)
	{
		CORE_ASSERT(m_Layout, "VertexLayout::push: layout was not initialized!");
		auto [type, count] = vertex_attrib_to_gl_enum(attribute);
		m_Layout->push_attrib(count, type, offset, m_BufferIndx);
	}

	void VertexLayout::set_index(uint32_t index)
	{
		CORE_ASSERT(m_Layout, "VertexLayout::set_index: layout was not initialized!");
		m_BufferIndx = index;
	}

	void VertexLayout::bind(const VertexLayout &layout)
	{
		CORE_ASSERT(layout.is_init(), "VertexLayout::bind: layout was not initialized!");

		if (s_GlobalBindingContext.layout == layout) return;
		s_GlobalBindingContext.layout = layout;
		gl_utils::bind_vertex_layout(layout.m_Layout);
	}

	//void VertexLayout::unbind()
	//{
	//	glBindVertexArray(0);
	//}

	size_t VertexLayout::hash() const
	{
		return std::hash<void *>()(m_Layout.get());
	}

	Shader::Shader(const ShaderCreateInfo &info)
		: m_Layout(info.layout)
	{
		gl_utils::GLShaderCreateInfo shaderInfo{};

		if (info.modules.size() == 1 && info.modules.at(0).second == ShaderType::COMPUTE) m_IsCompute = true;

		for (auto &module : info.modules) {
			shaderInfo.push_back({ module.first, shader_type_to_gl_enum(module.second) });
		}

		m_Shader = make_ref<gl_utils::GLShader>(shaderInfo);
	}

	void Shader::bind(const Shader &shader)
	{
		CORE_ASSERT(shader.is_init(), "Shader::bind: Shader was not initialized!");
		CORE_ASSERT(shader.m_Layout.is_init(), "Shader::bind: VertexLayout was not initialized!");

		if (s_GlobalBindingContext.shader == shader) return;
		s_GlobalBindingContext.shader = shader;
		gl_utils::bind_shader(shader.m_Shader);

		for (auto &pair : shader.m_UniformBuffers) {
			const Buffer &buff = pair.second;
			gl_utils::bind_uniform_buffer(pair.second.m_Buffer, shader.m_Shader->get_uniform_block_binding(pair.first));
		}

		for (auto &pair : shader.m_StorageBuffers) {
			const Buffer &buff = pair.second;
			gl_utils::bind_storage_buffer(pair.second.m_Buffer, shader.m_Shader->get_storage_block_binding(pair.first));
		}

		for (auto &pair : shader.m_Textures) {
			const Texture2D &tex = pair.second.texture;
			const TextureUsageBits usages = pair.second.usages;

			Texture2D::bind(tex, pair.second.bindingPoint, usages);
		}

		if (!shader.m_IsCompute) {
			VertexLayout::bind(shader.m_Layout);
		}
		//Render::bind_vertexlayout(shader.m_Layout);
	}

	void Shader::unbind()
	{
		s_GlobalBindingContext.shader = Shader();
		glUseProgram(0);
		//VertexLayout::unbind();
	}

	void Shader::dispatch(const Shader &shader, uint32_t nGroupsX, uint32_t nGroupsY, uint32_t nGroupsZ)
	{
		ATL_EVENT();
		Shader::bind(shader);
		glDispatchCompute(nGroupsX, nGroupsY, nGroupsZ);
	}

	Shader Shader::load_vert_frag(const std::string &vertexFile, const std::string &fragFile, const VertexLayout &layout)
	{
		ShaderCreateInfo info{};
		info.layout = layout;
		info.modules.push_back({ vertexFile, ShaderType::VERTEX });
		info.modules.push_back({ fragFile, ShaderType::FRAGMENT });
		return Shader(info);
	}

	Shader Shader::load_comp(const std::string &file)
	{
		ShaderCreateInfo info{};
		info.layout = VertexLayout::empty();
		info.modules.push_back({ file, ShaderType::COMPUTE });
		return Shader(info);
	}

#define IMPL_SHADER_FUNC(TYPE, FUNC_NAME) \
	void Shader::set(const std::string &name, TYPE value) \
	{ \
		CORE_ASSERT(m_Shader, "Shader::set_int: Shader was not initialized!"); \
		m_Shader->FUNC_NAME(name.c_str(), value); \
	}

	IMPL_SHADER_FUNC(int32_t, set_int);
	IMPL_SHADER_FUNC(const glm::ivec2 &, set_int2);
	IMPL_SHADER_FUNC(const glm::ivec3 &, set_int3);
	IMPL_SHADER_FUNC(const glm::ivec4 &, set_int4);

	void Shader::set(const std::string &name, int32_t *value, size_t count)
	{
		m_Shader->set_int_vec(name.c_str(), value, count);
	}

	IMPL_SHADER_FUNC(uint32_t, set_int);
	IMPL_SHADER_FUNC(const glm::uvec2 &, set_uint2);
	IMPL_SHADER_FUNC(const glm::uvec3 &, set_uint3);
	IMPL_SHADER_FUNC(const glm::uvec4 &, set_uint4);

	void Shader::set(const std::string &name, uint32_t *value, size_t count)
	{
		m_Shader->set_uint_vec(name.c_str(), value, count);
	}

	IMPL_SHADER_FUNC(float, set_float);
	IMPL_SHADER_FUNC(const glm::vec2 &, set_float2);
	IMPL_SHADER_FUNC(const glm::vec3 &, set_float3);
	IMPL_SHADER_FUNC(const glm::vec4 &, set_float4);

	void Shader::set(const std::string &name, float *value, size_t count)
	{
		m_Shader->set_float_vec(name.c_str(), value, count);
	}

	IMPL_SHADER_FUNC(const glm::mat3 &, set_mat3);
	IMPL_SHADER_FUNC(const glm::mat4 &, set_mat4);

	void Shader::bind(const std::string &name, const Buffer &buffer)
	{
		CORE_ASSERT(buffer.m_Types & (BufferType::UNIFORM | BufferType::STORAGE), "Shader::bind: buffer was not initialized as unifrom / storage buffer!");

		if (buffer.m_Types & BufferType::UNIFORM) {
			if (m_Shader->get_uniform_block_binding(name) == -1) CORE_WARN("Shader::bind: could not find Uniform Buffer: {}", name);
			m_UniformBuffers.insert_or_assign(name, buffer);
		}
		else if (buffer.m_Types & BufferType::STORAGE) {
			if (m_Shader->get_storage_block_binding(name) == -1) CORE_WARN("Shader::bind: could not find Storage Buffer: {}", name);
			m_StorageBuffers.insert_or_assign(name, buffer);
		}
	}

	void Shader::bind(const std::string &name, const Texture2D &texture, TextureUsageBits usages)
	{
		CORE_ASSERT(m_Shader->get_uniform_location(name) != -1, "Shader::bind: could not find uniform: {}", name);

		BoundTextureInfo info{};
		info.texture = texture;
		info.usages = usages;
		info.bindingPoint = m_Shader->get_int(name.c_str());
		m_Textures.insert_or_assign(name, info);
	}

	Buffer &Shader::get_uniform_buffer(const char *name)
	{
		auto it = m_UniformBuffers.find(name);
		CORE_ASSERT(it != m_UniformBuffers.end(), "Shader::get_uniform_buffer: could not find buffer");

		return it->second;
	}

	Buffer &Shader::get_storage_buffer(const char *name)
	{
		auto it = m_StorageBuffers.find(name);
		CORE_ASSERT(it != m_StorageBuffers.end(), "Shader::get_uniform_buffer: could not find buffer");

		return it->second;
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

	Buffer &Render::get_bound_index_buffer()
	{
		return s_GlobalBindingContext.indexBuffer;
	}

	Buffer &Render::get_bound_vertex_buffer(uint32_t index)
	{
		return s_GlobalBindingContext.vertexBuffers.at(index);
	}

}

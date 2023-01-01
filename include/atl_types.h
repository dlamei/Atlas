#pragma once

#include "imgui.h"

#include <glm/glm.hpp>

namespace gl_utils {
	class GLTexture2D;
	class GLFramebuffer;
	class GLBuffer;
	class GLShader;
	class GLVertexLayout;
	class GLShader;
}

namespace Atlas {
	class Texture2D;
	class Framebuffer;
	class Buffer;
	class Shader;
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


	class Framebuffer {
	public:

		using Attachment = Texture2D;

		Framebuffer() = default;
		Framebuffer(std::vector<Attachment> colorAttachments);

		static Framebuffer empty();

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

	enum class BufferUsage : uint32_t {
		STATIC_DRAW,
		DYNAMIC_DRAW,
	};


	namespace BufferType {
		enum Type : uint32_t {
			VERTEX = BIT(0),
			INDEX_U32 = BIT(1),
			UNIFORM = BIT(2),
		};
	}
	using BufferTypes = uint32_t;


	struct BufferCreateInfo {
		size_t size;
		BufferUsage usage;
		BufferTypes types;

		size_t stride;

		void *data;
	};

	class Buffer {
	public:

		Buffer() = default;
		Buffer(const BufferCreateInfo &info);

		template <typename T>
		static Buffer vertex(T *data, size_t count, BufferUsage usage = BufferUsage::STATIC_DRAW) {
			BufferCreateInfo info{};
			info.size = sizeof(T) * count;
			info.data = (void *)data;
			info.types = BufferType::VERTEX;
			info.usage = usage;
			info.stride = sizeof(T);

			return Buffer(info);
		}

		static Buffer index(uint32_t *data, size_t count, BufferUsage usage = BufferUsage::STATIC_DRAW) {
			BufferCreateInfo info{};
			info.size = sizeof(uint32_t) * count;
			info.data = (void *)data;
			info.types = BufferType::INDEX_U32;
			info.usage = usage;
			info.stride = sizeof(uint32_t);

			return Buffer(info);
		}

		void set_data(void *data, size_t size);

		size_t size();
		inline BufferTypes type() const { return m_Types; }
		inline bool is_init() const { return m_Buffer != nullptr; }

		static void bind_vertex(const Buffer &buffer, uint32_t index = 0);
		static void bind_index(const Buffer &buffer);

	private:
		Ref<gl_utils::GLBuffer> m_Buffer{ nullptr };
		BufferTypes m_Types;
		size_t m_Stride;
	};

	enum class VertexAttribute : uint32_t {
		NONE,
		INT, INT2, INT3, INT4,
		UINT, UINT2, UINT3, UINT4,
		FLOAT, FLOAT2, FLOAT3, FLOAT4,
	};

	template <typename T>
	struct vertex_attribute {
		static constexpr VertexAttribute attribute = VertexAttribute::NONE;
	};
	template <typename T>
	struct has_vertex_attribute {
		static constexpr bool value = false;
	};

#define DEFINE_ATTRIBUTE_TRAIT(TYPE, ENUM) \
	template <> \
	struct vertex_attribute<TYPE> { \
		static constexpr VertexAttribute attribute = VertexAttribute::ENUM; \
	}; \
	template <> \
	struct has_vertex_attribute<TYPE> { \
		static constexpr bool value = true; \
	}

	DEFINE_ATTRIBUTE_TRAIT(int, INT);
	DEFINE_ATTRIBUTE_TRAIT(glm::ivec1, INT);
	DEFINE_ATTRIBUTE_TRAIT(glm::ivec2, INT2);
	DEFINE_ATTRIBUTE_TRAIT(glm::ivec3, INT3);
	DEFINE_ATTRIBUTE_TRAIT(glm::ivec4, INT4);
	DEFINE_ATTRIBUTE_TRAIT(uint32_t, UINT);
	DEFINE_ATTRIBUTE_TRAIT(glm::uvec1, UINT);
	DEFINE_ATTRIBUTE_TRAIT(glm::uvec2, UINT2);
	DEFINE_ATTRIBUTE_TRAIT(glm::uvec3, UINT3);
	DEFINE_ATTRIBUTE_TRAIT(glm::uvec4, UINT4);
	DEFINE_ATTRIBUTE_TRAIT(float, FLOAT);
	DEFINE_ATTRIBUTE_TRAIT(glm::vec1, FLOAT);
	DEFINE_ATTRIBUTE_TRAIT(glm::vec2, FLOAT2);
	DEFINE_ATTRIBUTE_TRAIT(glm::vec3, FLOAT3);
	DEFINE_ATTRIBUTE_TRAIT(glm::vec4, FLOAT4);

	std::pair<uint32_t, uint32_t> vertex_attrib_to_gl_enum(VertexAttribute a); //enum, count

	class VertexLayout {
	public:

		VertexLayout() = default;

		static VertexLayout empty();

		template <typename T, typename U, typename... Args>
		static VertexLayout from(U T:: *member, Args&&... args) {
			VertexLayout layout = from(std::forward<Args>(args)...);
			layout.push(member);
			return layout;
		}

		template <typename T, typename U>
		static VertexLayout from(U T:: *member) {
			VertexLayout layout = empty();
			layout.push(member);
			return layout;
		}

		void push(uint32_t count, uint32_t gl_type, uint32_t offset);
		void set_index(uint32_t index);

		void bind() const;

		template <typename T, typename U>
		void push(VertexAttribute attribute, U T:: *member) {
			auto glVal = vertex_attrib_to_gl_enum(attribute);
			push(glVal.second, glVal.first, (uint32_t)offset_of(member));
		}

		template <typename T, typename U>
		void push(U T:: *member) {

			if (!has_vertex_attribute<U>::value) {
				CORE_WARN("VertexLayout::push: type has no vertex_attribute trait defined!");
				return;
			}

			VertexAttribute attribute = vertex_attribute<U>::attribute;
			auto glVal = vertex_attrib_to_gl_enum(attribute);
			push(glVal.second, glVal.first, (uint32_t)offset_of(member));
		}

		inline bool is_init() { return m_Layout != nullptr; }

	private:
		Ref<gl_utils::GLVertexLayout> m_Layout;
		uint32_t m_BufferIndx{ 0 };

		template<typename T, typename U> constexpr size_t offset_of(U T:: *member)
		{
			return (char *)&((T *)nullptr->*member) - (char *)nullptr;
		}
	};

	enum class ShaderType {
		VERTEX,
		FRAGMENT,
		COMPUTE,
	};

	struct ShaderCreateInfo {
		std::vector<std::pair<std::string, ShaderType>> modules;
		VertexLayout layout;
	};

	class Shader {
	public:

		Shader() = default;
		Shader(const ShaderCreateInfo &info);

		static void bind(const Shader &info);
		static void unbind();

		static Shader load(const char *vertexFile, const char *fragFile, const VertexLayout &layout);

		inline bool is_init() const { return m_Shader != nullptr; }

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

	private:
		Ref<gl_utils::GLShader> m_Shader;
		VertexLayout m_Layout;
	};
}
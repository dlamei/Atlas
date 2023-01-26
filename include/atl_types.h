#pragma once

#include "imgui.h"

#include <glm/glm.hpp>
#include <glm/gtx/type_trait.hpp>

namespace gl_utils {
	class GLTexture2D;
	class GLFramebuffer;
	class GLBuffer;
	class GLShader;
	class GLVertexLayout;
	class GLShader;
}

namespace Atlas {
	class Color;
	class Texture2D;
	class Framebuffer;
	class Buffer;
	class Shader;
	class VertexLayout;
}

namespace ImGui {

	void Image(const Atlas::Texture2D &texture, const ImVec2 &size,
		const ImVec2 &uv0 = ImVec2(0, 1), const ImVec2 &uv1 = ImVec2(1, 0),
		const ImVec4 &tint_col = ImVec4(1, 1, 1, 1), const ImVec4 &border_col = ImVec4(0, 0, 0, 0));

	bool ImageButton(const Atlas::Texture2D &texture, const ImVec2 &size,
		const ImVec2 &uv0 = ImVec2(0, 1), const ImVec2 &uv1 = ImVec2(1, 0),
		int frame_padding = -1, const ImVec4 &bg_col = ImVec4(0, 0, 0, 0),
		const ImVec4 &tint_col = ImVec4(1, 1, 1, 1));
}

namespace Atlas {

	namespace Random {
		void init();

		template<typename T>
		struct is_randomizable {
			static constexpr bool value = std::is_integral_v<T> || std::is_floating_point_v<T> || glm::type<T>::is_vec;
		};

		template<typename T>
		const constexpr bool is_randomizable_v = is_randomizable<T>::value;

		int64_t uniform_integer();
		double uniform_real();

		template<typename T, std::enable_if_t<!is_randomizable_v<T>>...>
		void get() {
			CORE_ASSERT(false, "Random::get is not defined for type: {}", typeid(T).name())
		}

		template<typename T, std::enable_if_t<!is_randomizable_v<T>>...>
		void get(T min, T max) {
			CORE_ASSERT(false, "Random::get(min, max) is not defined for type: {}", typeid(T).name())
		}

		//template<typename T>
		//typename std::enable_if_t<std::is_integral_v<T>, T> get() {
		//	return (T)uniform_integer();
		//}

		template<typename T, std::enable_if_t<std::is_integral_v<T>>...>
		T get() {
			return (T)uniform_integer();
		}

		template<typename T, std::enable_if_t<std::is_floating_point_v<T>>...>
		T get() {
			return (T)uniform_real();
		}

		template<typename T, std::enable_if_t<std::is_integral_v<T>>...>
		T get(T min, T max) {
			T range = max - min + 1;
			return min + (get<T>() % range + range) % range;
		}

		template<typename T, std::enable_if_t<std::is_floating_point_v<T>>...>
		T get(T min, T max) {
			return get<T>() * (max - min) + min;
		}

		template<typename T, std::enable_if_t<std::is_floating_point_v<T> || std::is_integral_v<T>>...>
		T get(T max) {
			return get<T>(0, max);
		}

		template<typename T, std::enable_if_t<glm::type<T>::is_vec>...>
		T get() {
			T value{};
			const constexpr glm::length_t length = glm::type<T>::components;
			for (glm::length_t i = 0; i < length; i++) {
				value[i] = get<T::value_type>();
			}
			return value;
		}

		template<typename T, std::enable_if_t<glm::type<T>::is_vec>...>
		T get(typename T::value_type min, typename T::value_type max) {
			T value{};
			const constexpr glm::length_t length = glm::type<T>::components;
			for (glm::length_t i = 0; i < length; i++) {
				value[i] = get<T::value_type>(min, max);
			}
			return value;
		}
	}

	namespace Render {
		Buffer &get_bound_index_buffer();
		Buffer &get_bound_vertex_buffer(uint32_t index = 0);
	}

	class Color {
	public:
		Color();
		Color(uint8_t value);
		Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
		Color(uint8_t r, uint8_t g, uint8_t b);

		static Color from_norm(glm::vec3 val);
		static Color from_norm(glm::vec4 val);

		inline uint8_t red() const;
		inline uint8_t green() const;
		inline uint8_t blue() const;
		inline uint8_t alpha() const;

		glm::vec4 normalized();

		explicit operator uint32_t() const;

	private:
		uint32_t m_Data;
	};

	inline bool operator==(const Color &c1, const Color &c2) {
		return (uint32_t)c1 == (uint32_t)c2;
	}

	inline bool operator!=(const Color &c1, const Color &c2) {
		return (uint32_t)c1 != (uint32_t)c2;
	}

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

	namespace TextureUsage {
		enum _ : uint32_t {
			SAMPLER = 1 << 0,
			READ = 1 << 1,
			WRITE = 1 << 2,
		};
	}
	using TextureUsageBits = uint32_t;

	class Texture2D {
	public:

		Texture2D() = default;
		Texture2D(const Texture2DCreateInfo &info);

		static Texture2D rgba(uint32_t width, uint32_t height, TextureFilter filter = TextureFilter::LINEAR);
		static Texture2D rgb(uint32_t width, uint32_t height, TextureFilter filter = TextureFilter::LINEAR);
		static Texture2D depth(uint32_t width, uint32_t height, TextureFilter filter = TextureFilter::LINEAR);
		static Texture2D depth_stencil(uint32_t width, uint32_t height, TextureFilter filter = TextureFilter::LINEAR);
		static std::optional<Texture2D> load(const char *filePath, TextureFilter filter = TextureFilter::LINEAR);

		static void bind(const Texture2D &texture, uint32_t indx = 0, TextureUsageBits usage = TextureUsage::SAMPLER);
		static void unbind(uint32_t index);
		//static void bind_image(const Texture2D &texture, uint32_t unit);

		void set_data(const Color *data, size_t size) const;
		void fill(const Color fillColor) const;
		void fill_random();
		void fill_random_greyscale();

		uint32_t width() const;
		uint32_t height() const;
		bool has_mipmap() const;
		inline ColorFormat format() const { return m_Format; }
		inline bool is_init() const { return m_Texture != nullptr; }

		friend bool operator==(const Texture2D &t1, const Texture2D &t2);
		friend bool operator!=(const Texture2D &t1, const Texture2D &t2);

		friend void ImGui::Image(const Atlas::Texture2D &texture, const ImVec2 &size,
			const ImVec2 &uv0, const ImVec2 &uv1, const ImVec4 &tint_col, const ImVec4 &border_col);

		friend bool ImGui::ImageButton(const Atlas::Texture2D &texture, const ImVec2 &size,
			const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col,
			const ImVec4 &tint_col);

		size_t hash() const;


	private:
		ColorFormat m_Format{ 0 };
		Ref<gl_utils::GLTexture2D> m_Texture{ nullptr };

		friend class Framebuffer;
	};

	using FramebufferAttachment = Texture2D;

	struct FramebufferCreateInfo {
		std::vector<FramebufferAttachment> colorAttachments;
		FramebufferAttachment depthAttachments;
	};

	class Framebuffer {
	public:


		Framebuffer() = default;
		Framebuffer(const FramebufferCreateInfo &info);
		//TODO: FramebufferCreateInfo

		static Framebuffer empty();

		static void bind(const Framebuffer &framebuffer);
		static void unbind();

		void set_color_attachment(const Texture2D &texture, uint32_t index);
		void set_depth_stencil_texture(const Texture2D &texture);

		const Texture2D &get_color_attachment(uint32_t index);

		inline uint32_t width() { return m_Width; }
		inline uint32_t height() { return m_Height; }

		inline bool is_init() const { return m_Framebuffer != nullptr; }

		friend bool operator==(const Framebuffer &f1, const Framebuffer &f2);
		friend bool operator!=(const Framebuffer &f1, const Framebuffer &f2);

		size_t hash() const;

	private:
		Ref<gl_utils::GLFramebuffer> m_Framebuffer{ nullptr };
		std::unordered_map<uint32_t, Texture2D> m_ColorTextures;
		Texture2D m_DepthStencilTexture;

		uint32_t m_Width{ 0 };
		uint32_t m_Height{ 0 };
	};

	enum class BufferUsage : uint32_t {
		STATIC,
		DYNAMIC,
	};


	namespace BufferType {
		enum _ : uint32_t {
			VERTEX = 1 << 0,
			INDEX_U32 = 1 << 1,
			UNIFORM = 1 << 2,
			STORAGE = 1 << 3,
		};
	}
	using BufferTypeBits = uint32_t;


	struct BufferCreateInfo {
		size_t size;
		BufferUsage usage;
		BufferTypeBits types;

		size_t stride;
		void *data;
	};

	class Buffer {
	public:

		Buffer() = default;
		Buffer(const BufferCreateInfo &info);

		template <typename T>
		static Buffer vertex(size_t count, BufferUsage usage = BufferUsage::STATIC) {
			BufferCreateInfo info{};
			info.size = sizeof(T) * count;
			info.data = nullptr;
			info.types = BufferType::VERTEX;
			info.usage = usage;
			info.stride = sizeof(T);
			return Buffer(info);
		}

		template <typename T>
		static Buffer uniform(const T &value, BufferUsage usage = BufferUsage::STATIC) {
			BufferCreateInfo info{};
			info.size = sizeof(T);
			info.data = (void *)&value;
			info.types = BufferType::UNIFORM;
			info.usage = usage;
			info.stride = sizeof(T);
			return Buffer(info);
		}

		template <typename T>
		static Buffer storage(const T &value, BufferUsage usage = BufferUsage::STATIC) {
			BufferCreateInfo info{};
			info.size = sizeof(T);
			info.data = (void *)&value;
			info.types = BufferType::STORAGE;
			info.usage = usage;
			info.stride = sizeof(T);

			return Buffer(info);
		}

		static Buffer create(BufferTypeBits types, void *data, size_t size, BufferUsage usage = BufferUsage::STATIC, size_t stride = 0);

		static Buffer uniform(void *data, size_t size, BufferUsage usage = BufferUsage::STATIC);
		static Buffer storage(void *data, size_t size, BufferUsage usage = BufferUsage::STATIC);

		static Buffer index(size_t count, BufferUsage usage = BufferUsage::STATIC);

		void set_data(void *data, size_t size);

		template <typename T>
		void set_data(const T &value) {
			set_data((void *)&value, sizeof(T));
		}

		std::vector<void *> get_data();

		size_t size() const;
		inline BufferTypeBits type() const { return m_Types; }
		inline bool is_init() const { return m_Buffer != nullptr; }

		static void bind_vertex(const Buffer &buffer, uint32_t index = 0);
		static void unbind_vertex(uint32_t index = 0);
		static void bind_index(const Buffer &buffer);
		static void unbind_index();
		static void map_write(const Buffer &buffer, std::function<void(void *)> func);

		friend bool operator==(const Buffer &b1, const Buffer &b2);
		friend bool operator!=(const Buffer &b1, const Buffer &b2);

		size_t hash() const;

	private:
		Ref<gl_utils::GLBuffer> m_Buffer{ nullptr };
		BufferTypeBits m_Types{ 0 };
		size_t m_Stride{ 0 };

		friend class Shader;
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

#define DEFINE_VERTEX_ATTRIBUTE_TRAIT(TYPE, ENUM) \
	template <> \
	struct vertex_attribute<TYPE> { \
		static constexpr VertexAttribute attribute = VertexAttribute::ENUM; \
	}; \
	template <> \
	struct has_vertex_attribute<TYPE> { \
		static constexpr bool value = true; \
	}

	DEFINE_VERTEX_ATTRIBUTE_TRAIT(int, INT);
	DEFINE_VERTEX_ATTRIBUTE_TRAIT(glm::ivec1, INT);
	DEFINE_VERTEX_ATTRIBUTE_TRAIT(glm::ivec2, INT2);
	DEFINE_VERTEX_ATTRIBUTE_TRAIT(glm::ivec3, INT3);
	DEFINE_VERTEX_ATTRIBUTE_TRAIT(glm::ivec4, INT4);
	DEFINE_VERTEX_ATTRIBUTE_TRAIT(uint32_t, UINT);
	DEFINE_VERTEX_ATTRIBUTE_TRAIT(glm::uvec1, UINT);
	DEFINE_VERTEX_ATTRIBUTE_TRAIT(glm::uvec2, UINT2);
	DEFINE_VERTEX_ATTRIBUTE_TRAIT(glm::uvec3, UINT3);
	DEFINE_VERTEX_ATTRIBUTE_TRAIT(glm::uvec4, UINT4);
	DEFINE_VERTEX_ATTRIBUTE_TRAIT(float, FLOAT);
	DEFINE_VERTEX_ATTRIBUTE_TRAIT(glm::vec1, FLOAT);
	DEFINE_VERTEX_ATTRIBUTE_TRAIT(glm::vec2, FLOAT2);
	DEFINE_VERTEX_ATTRIBUTE_TRAIT(glm::vec3, FLOAT3);
	DEFINE_VERTEX_ATTRIBUTE_TRAIT(glm::vec4, FLOAT4);

	class VertexLayout {
	public:

		VertexLayout() = default;

		static VertexLayout empty();

		template <typename T, typename U, typename... Args>
		static VertexLayout from(U T:: *member, Args&&... args) {
			VertexLayout layout = empty();
			layout.push(member);
			from_rec(layout, std::forward<Args>(args)...);
			return layout;
		}

		template <typename T, typename U>
		static VertexLayout from(U T:: *member) {
			VertexLayout layout = empty();
			layout.push(member);
			return layout;
		}

		void push(VertexAttribute attribute, uint32_t offset);
		void set_index(uint32_t index);

		static void bind(const VertexLayout &layout);
		//static void unbind();

		template <typename T, typename U>
		void push(VertexAttribute attribute, U T:: *member) {
			push(attribute, (uint32_t)offset_of(member));
		}

		template <typename T, typename U>
		void push(U T:: *member) {

			if (!has_vertex_attribute<U>::value) {
				CORE_WARN("VertexLayout::push: type has no vertex_attribute trait defined!");
				return;
			}

			VertexAttribute attribute = vertex_attribute<U>::attribute;
			push(attribute, (uint32_t)offset_of(member));
		}

		inline bool is_init() const { return m_Layout != nullptr; }

		friend bool operator==(const VertexLayout &v1, const VertexLayout &v2);
		friend bool operator!=(const VertexLayout &v1, const VertexLayout &v2);

		size_t hash() const;

	private:
		Ref<gl_utils::GLVertexLayout> m_Layout;
		uint32_t m_BufferIndx{ 0 };

		template <typename T, typename U, typename... Args>
		static void from_rec(VertexLayout &layout, U T:: *member, Args&&... args) {
			layout.push(member);
			from_rec(layout, std::forward<Args>(args)...);
		}

		template <typename T, typename U>
		static void from_rec(VertexLayout &layout, U T:: *member) {
			layout.push(member);
		}

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

		static void bind(const Shader &shader);
		static void unbind();
		static void dispatch(const Shader &shader, uint32_t nGroupsX, uint32_t nGroupsY, uint32_t nGroupsZ);

		static Shader load_vert_frag(const std::string &vertexFile, const std::string &fragFile, const VertexLayout &layout);
		static Shader load_comp(const std::string &file);

		inline bool is_init() const { return m_Shader != nullptr; }

		template <typename T>
		void bind(const std::string &name, T value)
		{
			CORE_ASSERT(false, "Shader::bind not defined for type: {}", typeid(T).name());
		}

		void set(const std::string &name, int32_t value);
		void set(const std::string &name, const glm::ivec2 &value);
		void set(const std::string &name, const glm::ivec3 &value);
		void set(const std::string &name, const glm::ivec4 &value);
		void set(const std::string &name, int32_t *value, size_t count);

		void set(const std::string &name, uint32_t value);
		void set(const std::string &name, const glm::uvec2 &value);
		void set(const std::string &name, const glm::uvec3 &value);
		void set(const std::string &name, const glm::uvec4 &value);
		void set(const std::string &name, uint32_t *value, size_t count);

		void set(const std::string &name, float value);
		void set(const std::string &name, const glm::vec2 &value);
		void set(const std::string &name, const glm::vec3 &value);
		void set(const std::string &name, const glm::vec4 &value);
		void set(const std::string &name, float *value, size_t count);

		void set(const std::string &name, const glm::mat3 &value);
		void set(const std::string &name, const glm::mat4 &value);

		void bind(const std::string &name, const Buffer &buffer);
		void bind(const std::string &name, const Texture2D &texture, TextureUsageBits usages = TextureUsage::SAMPLER);

		Buffer &get_uniform_buffer(const char *name);
		Buffer &get_storage_buffer(const char *name);

		inline VertexLayout get_layout() { return m_Layout; }

		friend bool operator==(const Shader &s1, const Shader &s2);
		friend bool operator!=(const Shader &s1, const Shader &s2);

		size_t hash() const;

	private:

		struct BoundTextureInfo {
			Texture2D texture;
			TextureUsageBits usages;
			int bindingPoint;
		};

		Ref<gl_utils::GLShader> m_Shader;
		std::unordered_map<std::string, Buffer> m_UniformBuffers;
		std::unordered_map<std::string, Buffer> m_StorageBuffers;
		std::unordered_map<std::string, BoundTextureInfo> m_Textures;
		bool m_IsCompute{ false };

		VertexLayout m_Layout;
	};

}
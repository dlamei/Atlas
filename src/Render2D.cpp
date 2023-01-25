#include "Render2D.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>

#include "RenderApi.h"
#include "camera.h"
#include "atl_types.h"

namespace Atlas::Render2D {

	struct GPUCameraData {
		glm::mat4 viewProj;
	};

	struct RenderData {
		static const uint32_t MAX_VERTICES = 4 * 5000;
		static const uint32_t MAX_INDICES = 6 * 5000;
		static const uint32_t MAX_TEXTURE_SLOTS = 32;

		bool init{ false };

		glm::mat4 viewProj;
		Shader shader;

		Buffer vertexBuffer;
		Buffer indexBuffer;

		std::array<Vertex, MAX_VERTICES> vertices{};
		std::array<uint32_t, MAX_INDICES> indices{};

		std::array<Texture2D, MAX_TEXTURE_SLOTS> textures{};
		uint32_t textureIndex{ 1 };

		uint32_t vertexCount{ 0 };
		uint32_t indexCount{ 0 };

		Vertex *vertexPtr{ nullptr };
		uint32_t *indexPtr{ nullptr };

		Texture2D whiteTexture;

		RenderStats stats;
	};

	static RenderData s_RenderData;

	void init()
	{
		if (s_RenderData.init) {
			CORE_WARN("Render2D::init: already initialized!");
			return;
		}

		s_RenderData.init = true;

		s_RenderData.whiteTexture = Texture2D::rgba(1, 1);
		Color c = { 255, 255, 255 };
		s_RenderData.whiteTexture.set_data(&c, 1);
		s_RenderData.textures[0] = s_RenderData.whiteTexture;

		auto layout = VertexLayout::from(&Vertex::pos, &Vertex::uv, &Vertex::color, &Vertex::texID, &Vertex::isEllipse);
		s_RenderData.shader = Shader::load_vert_frag("assets/shaders/default.vert", "assets/shaders/default.frag", layout);

		s_RenderData.vertexBuffer = Buffer::vertex<Vertex>(RenderData::MAX_VERTICES, BufferUsage::DYNAMIC);
		s_RenderData.indexBuffer = Buffer::index(RenderData::MAX_INDICES, BufferUsage::DYNAMIC);

		//TODO: generate buffer in shader?
		Buffer cameraBuffer = Buffer::uniform(glm::ortho(-1, 1, -1, 1), BufferUsage::DYNAMIC);
		s_RenderData.shader.bind("CameraBuffer", cameraBuffer);

		s_RenderData.vertexPtr = s_RenderData.vertices.data();
		s_RenderData.indexPtr = s_RenderData.indices.data();

		int textureSlots[RenderData::MAX_TEXTURE_SLOTS];
		for (int i = 0; i < RenderData::MAX_TEXTURE_SLOTS; i++) textureSlots[i] = i;
		s_RenderData.shader.set("uTextureSlots[0]", textureSlots, RenderData::MAX_TEXTURE_SLOTS);
	}

	int push_texture(const Texture2D &texture) {

		for (uint32_t i = 0; i < s_RenderData.textureIndex; i++) {
			if (s_RenderData.textures.at(i) == texture) return i;
		}

		s_RenderData.textures.at(s_RenderData.textureIndex) = texture;
		return s_RenderData.textureIndex++;
	}

	inline void push_local_vertex(const Vertex &v)
	{
		*s_RenderData.vertexPtr = v;
		s_RenderData.vertexPtr++;
	}

	inline void push_local_index(uint32_t index)
	{
		*s_RenderData.indexPtr = s_RenderData.vertexCount + index;
		s_RenderData.indexPtr++;
	}

	void rect_impl(const glm::vec2 &pos, const glm::vec2 &size, const Texture2D &texture, Color tint, bool isEllipse) {
		const uint32_t vertexCount = 4;
		const uint32_t indexCount = 6;

		if (s_RenderData.vertexCount + vertexCount >= RenderData::MAX_VERTICES) flush();
		if (s_RenderData.indexCount + indexCount >= RenderData::MAX_INDICES) flush();
		if (s_RenderData.textureIndex + 1 >= RenderData::MAX_TEXTURE_SLOTS) flush();

		int texID = push_texture(texture);

		auto normColor = tint.normalized();

		Vertex v{};
		v.color = normColor;
		v.texID = texID;
		v.isEllipse = (int)isEllipse;

		v.pos = pos;
		v.uv = glm::vec2(0, 0);
		push_local_vertex(v);

		v.pos = { pos.x + size.x, pos.y };
		v.uv = glm::vec2(1, 0);
		push_local_vertex(v);

		v.pos = { pos.x + size.x, pos.y + size.y };
		v.uv = glm::vec2(1, 1);
		push_local_vertex(v);

		v.pos = { pos.x, pos.y + size.y };
		v.uv = glm::vec2(0, 1);
		push_local_vertex(v);

		push_local_index(0);
		push_local_index(1);
		push_local_index(2);
		push_local_index(2);
		push_local_index(3);
		push_local_index(0);

		s_RenderData.vertexCount += vertexCount;
		s_RenderData.indexCount += indexCount;
		s_RenderData.stats.triangleCount += 2;
	}

	void tri_impl(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3, Color color) {
		const uint32_t vertexCount = 3;
		const uint32_t indexCount = 3;

		if (s_RenderData.vertexCount + vertexCount >= RenderData::MAX_VERTICES) flush();
		if (s_RenderData.indexCount + indexCount >= RenderData::MAX_INDICES) flush();

		int texID = 0;
		auto normColor = color.normalized();

		Vertex v{};
		v.color = normColor;
		v.texID = texID;
		v.uv = glm::vec2(0, 0);
		v.isEllipse = 0;

		v.pos = p1;
		push_local_vertex(v);
		v.pos = p2;
		push_local_vertex(v);
		v.pos = p3;
		push_local_vertex(v);

		push_local_index(0);
		push_local_index(1);
		push_local_index(2);

		s_RenderData.vertexCount += vertexCount;
		s_RenderData.indexCount += indexCount;
		s_RenderData.stats.triangleCount++;
	}

	void rect(const glm::vec2 &pos, const glm::vec2 &size, const Texture2D &texture)
	{
		rect_impl(pos, size, texture, { 255 }, false);
	}

	void rect(const glm::vec2 &pos, const glm::vec2 &size, Color color)
	{
		rect_impl(pos, size, s_RenderData.whiteTexture, color, false);
	}

	void rect(const glm::vec2 &pos, const glm::vec2 &size, const Texture2D &texture, Color tint)
	{
		rect_impl(pos, size, texture, tint, false);
	}

	void square(const glm::vec2 &pos, float size, const Texture2D &texture)
	{
		rect_impl(pos, { size, size }, texture, { 255 }, false);
	}

	void square(const glm::vec2 &pos, float size, Color color)
	{
		rect_impl(pos, { size, size }, s_RenderData.whiteTexture, color, false);
	}

	void square(const glm::vec2 &pos, float size, const Texture2D &texture, Color tint)
	{
		rect_impl(pos, { size, size }, texture, tint, false);
	}

	void ellipse(const glm::vec2 &center, const glm::vec2 &size, Color color)
	{
		glm::vec2 s(size.x * 2, size.y * 2);
		rect_impl(center - s / 2.0f, s, s_RenderData.whiteTexture, { 255 }, 1);
	}

	void ellipse(const glm::vec2 &center, const glm::vec2 &size, const Texture2D &texture)
	{
		glm::vec2 s(size.x * 2, size.y * 2);
		rect_impl(center - s / 2.0f, s, texture, { 255 }, true);
	}

	void ellipse(const glm::vec2 &center, const glm::vec2 &size, const Texture2D &texture, Color tint)
	{
		glm::vec2 s(size.x * 2, size.y * 2);
		rect_impl(center - s / 2.0f, s, texture, tint, true);
	}

	void circle(const glm::vec2 &center, float radius, Color color)
	{
		glm::vec2 size(radius * 2, radius * 2);
		rect_impl(center - size / 2.0f, size, s_RenderData.whiteTexture, { 255 }, true);
	}

	void circle(const glm::vec2 &center, float radius, const Texture2D &texture)
	{
		glm::vec2 size(radius * 2, radius * 2);
		rect_impl(center - size / 2.0f, size, texture, { 255 }, true);
	}

	void circle(const glm::vec2 &center, float radius, const Texture2D &texture, Color color)
	{
		glm::vec2 size(radius * 2, radius * 2);
		rect_impl(center - size / 2.0f, size, texture, color, true);
	}

	void tri(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3, Color color)
	{
		tri_impl(p1, p2, p3, color);
	}

	void reset() {
		s_RenderData.vertexPtr = s_RenderData.vertices.data();
		s_RenderData.indexPtr = s_RenderData.indices.data();
		s_RenderData.vertexCount = 0;
		s_RenderData.indexCount = 0;
		s_RenderData.textureIndex = 1;
	}

	void flush() {
		ATL_EVENT();
		s_RenderData.stats.drawCalls++;
		s_RenderData.vertexBuffer.set_data(s_RenderData.vertices.data(), s_RenderData.vertexCount * sizeof(Vertex));
		s_RenderData.indexBuffer.set_data(s_RenderData.indices.data(), s_RenderData.indexCount * sizeof(uint32_t));

		Shader::bind(s_RenderData.shader);
		Buffer::bind_index(s_RenderData.indexBuffer);
		Buffer::bind_vertex(s_RenderData.vertexBuffer);

		for (uint32_t i = 0; i < s_RenderData.textureIndex; i++) {
			Texture2D::bind(s_RenderData.textures.at(i), i);
		}

		Render::draw_indexed(s_RenderData.indexCount);
		Render::flush();
		reset();
	}

	void set_camera(const Camera &camera)
	{
		if (camera.get_view_projection() == s_RenderData.viewProj) return;

		s_RenderData.viewProj = camera.get_view_projection();
		s_RenderData.shader.get_uniform_buffer("CameraBuffer").set_data(s_RenderData.viewProj);
	}

	void set_view_proj(const glm::mat4 &viewProj)
	{
		if (viewProj == s_RenderData.viewProj) return;
		s_RenderData.viewProj = viewProj;
		s_RenderData.shader.get_uniform_buffer("CameraBuffer").set_data(s_RenderData.viewProj);
	}

	void reset_stats()
	{
		s_RenderData.stats = RenderStats{};
	}

	RenderStats get_stats()
	{
		return s_RenderData.stats;
	}
}
#include "Render2D.h"

#include <glm/gtx/transform.hpp>

#include "RenderApi.h"
#include "camera.h"
#include "atl_types.h"


namespace Atlas::Render2D {

	struct GPUCameraData {
		glm::mat4 viewProj;
	};

	struct RenderData {
		static const uint32_t MAX_VERTICES = 4 * 2;
		static const uint32_t MAX_INDICES = 6 * 2;

		bool init{ false };

		glm::mat4 viewProj;
		Shader shader;
		VertexLayout layout;

		Buffer vertexBuffer;
		Buffer indexBuffer;

		std::array<Vertex, MAX_VERTICES> vertices;
		std::array<uint32_t, MAX_INDICES> indices;

		uint32_t vertexCount{ 0 };
		uint32_t indexCount{ 0 };

		Vertex *vertexPtr;
		uint32_t *indexPtr;

		Texture2D whiteTexture;
	};

	static RenderData s_RenderData;

	void init()
	{
		if (s_RenderData.init) {
			CORE_WARN("Render2D::init: already initialized!");
			return;
		}

		s_RenderData.init = true;

		s_RenderData.vertexPtr = s_RenderData.vertices.data();
		s_RenderData.indexPtr = s_RenderData.indices.data();

		s_RenderData.whiteTexture = Texture2D::color(1, 1);
		{
			Color c = { 255 };
			s_RenderData.whiteTexture.set_data(&c);
		}

		s_RenderData.layout = VertexLayout::from(&Vertex::pos, &Vertex::uv, &Vertex::color);
		s_RenderData.shader = Shader::load("assets/shaders/default.vert", "assets/shaders/default.frag", s_RenderData.layout);
		s_RenderData.shader.set("tex", 0);

		s_RenderData.vertexBuffer = Buffer::vertex<Vertex>(RenderData::MAX_VERTICES, BufferUsage::DYNAMIC);
		s_RenderData.indexBuffer = Buffer::index(RenderData::MAX_INDICES, BufferUsage::DYNAMIC);

		Buffer cameraBuffer = Buffer::uniform(glm::ortho(-1, 1, -1, 1), BufferUsage::DYNAMIC);
		s_RenderData.shader.set("CameraBuffer", cameraBuffer);
	}

	void rect(const glm::vec2 &pos, const glm::vec2 &size, Color color)
	{
		if (s_RenderData.vertexCount + 4 > RenderData::MAX_VERTICES) flush();
		if (s_RenderData.indexCount + 6 > RenderData::MAX_INDICES) flush();

		auto normColor = color.normalized();

		s_RenderData.vertexPtr->pos = glm::vec3(pos, 0);
		s_RenderData.vertexPtr->uv = glm::vec2(0, 0);
		s_RenderData.vertexPtr->color = normColor;
		s_RenderData.vertexPtr++;

		s_RenderData.vertexPtr->pos = { pos.x + size.x, pos.y, 0 };
		s_RenderData.vertexPtr->uv = glm::vec2(1, 0);
		s_RenderData.vertexPtr->color = normColor;
		s_RenderData.vertexPtr++;

		s_RenderData.vertexPtr->pos = { pos.x + size.x, pos.y + size.y, 0 };
		s_RenderData.vertexPtr->uv = glm::vec2(1, 1);
		s_RenderData.vertexPtr->color = normColor;
		s_RenderData.vertexPtr++;

		s_RenderData.vertexPtr->pos = { pos.x, pos.y + size.y, 0 };
		s_RenderData.vertexPtr->uv = glm::vec2(0, 1);
		s_RenderData.vertexPtr->color = normColor;
		s_RenderData.vertexPtr++;

		*(s_RenderData.indexPtr++) = s_RenderData.vertexCount + 0;
		*(s_RenderData.indexPtr++) = s_RenderData.vertexCount + 1;
		*(s_RenderData.indexPtr++) = s_RenderData.vertexCount + 2;
		*(s_RenderData.indexPtr++) = s_RenderData.vertexCount + 2;
		*(s_RenderData.indexPtr++) = s_RenderData.vertexCount + 3;
		*(s_RenderData.indexPtr++) = s_RenderData.vertexCount + 0;

		s_RenderData.vertexCount += 4;
		s_RenderData.indexCount += 6;
	}

	void flush() {
		s_RenderData.vertexBuffer.set_data(s_RenderData.vertices.data(), s_RenderData.vertexCount * sizeof(Vertex));
		s_RenderData.indexBuffer.set_data(s_RenderData.indices.data(), s_RenderData.indexCount * sizeof(uint32_t));

		Shader::bind(s_RenderData.shader);
		Buffer::bind_index(s_RenderData.indexBuffer);
		Buffer::bind_vertex(s_RenderData.vertexBuffer);
		Texture2D::bind(s_RenderData.whiteTexture);

		Render::draw_indexed(s_RenderData.indexCount);

		s_RenderData.vertexPtr = s_RenderData.vertices.data();
		s_RenderData.indexPtr = s_RenderData.indices.data();
		s_RenderData.vertexCount = 0;
		s_RenderData.indexCount = 0;
	}

	void set_camera(const Camera &camera)
	{
		if (camera.get_view_projection() == s_RenderData.viewProj) return;

		s_RenderData.viewProj = camera.get_view_projection();
		s_RenderData.shader.get_uniform_buffer("CameraBuffer").set_data(s_RenderData.viewProj);
	}
}
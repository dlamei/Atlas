#include "Render2D.h"

#include <glm/gtx/transform.hpp>

#include "RenderApi.h"

namespace Atlas::Render2D {

	struct GPUCameraData {
		glm::mat4 viewProj;
	};

	struct RenderData {
		bool init{ false };

		Shader defaultShader;
		Buffer vertexBuffer;
		Buffer indexBuffer;
	};

	static RenderData s_RenderData;

	void init()
	{
		if (s_RenderData.init) {
			CORE_WARN("Render2D::init: already initialized!");
			return;
		}

		s_RenderData.init = true;

		VertexLayout layout = VertexLayout::from(&Vertex::pos, &Vertex::uv);
		s_RenderData.defaultShader = Shader::load("assets/shaders/default.vert", "assets/shaders/default.frag", layout);

		Vertex vertices[] =
		{
			{{0, 0, 0}, {0, 0}},
			{{0, 1, 0}, {0, 1}},
			{{1, 1, 0}, {1, 1}},
			{{1, 0, 0}, {1, 0}},
		};
		uint32_t indices[] =
		{
			0, 1, 2,
			0, 2, 3
		};

		s_RenderData.vertexBuffer = Buffer::vertex(vertices, 4, BufferUsage::DYNAMIC);
		s_RenderData.indexBuffer = Buffer::index(indices, 6, BufferUsage::DYNAMIC);

		Buffer cameraBuffer = Buffer::uniform(glm::ortho(-1, 1, -1, 1), BufferUsage::DYNAMIC);
		s_RenderData.defaultShader.set("CameraData", cameraBuffer);
	}

	void test_draw()
	{
		Buffer::bind_index(s_RenderData.indexBuffer);
		Buffer::bind_vertex(s_RenderData.vertexBuffer);
		Shader::bind(s_RenderData.defaultShader);

		RenderApi::draw_indexed();
	}

}
#pragma once

#include <glm/glm.hpp>

#include "atl_types.h"
#include "camera.h"

namespace Atlas::Render2D {

	struct RenderStats {
		uint32_t drawCalls = 0;
		uint32_t triangleCount = 0;
	};

	struct Vertex {
		glm::vec2 pos;
		glm::vec2 uv;
		glm::vec4 color;
		int texID;
		int isEllipse;
	};

	void init();

	void rect(const glm::vec2 &pos, const glm::vec2 &size, const Texture2D &texture);
	void rect(const glm::vec2 &pos, const glm::vec2 &size, RGBA color);
	void rect(const glm::vec2 &pos, const glm::vec2 &size, const Texture2D &texture, RGBA tint);

	void square(const glm::vec2 &pos, float size, const Texture2D &texture);
	void square(const glm::vec2 &pos, float size, RGBA color);
	void square(const glm::vec2 &pos, float size, const Texture2D &texture, RGBA tint);

	void ellipse(const glm::vec2 &center, const glm::vec2 &size, RGBA color);
	void ellipse(const glm::vec2 &center, const glm::vec2 &size, const Texture2D &texture);
	void ellipse(const glm::vec2 &center, const glm::vec2 &size, const Texture2D &texture, RGBA tint);

	void circle(const glm::vec2 &center, float radius, RGBA color);
	void circle(const glm::vec2 &center, float radius, const Texture2D &texture);
	void circle(const glm::vec2 &center, float radius, const Texture2D &texture, RGBA color);

	void tri(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3, RGBA tint);

	void flush();

	void set_camera(const Camera &camera);
	void set_view_proj(const glm::mat4 &viewProj);

	void reset_stats();
	RenderStats get_stats();

}
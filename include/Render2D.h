#pragma once

#include <glm/glm.hpp>

#include "atl_types.h"
#include "camera.h"

namespace Atlas::Render2D {

	struct RenderStats {
		uint32_t drawCalls = 0;
		uint32_t trisDrawn = 0;
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
	void rect(const glm::vec2 &pos, const glm::vec2 &size, Color color);
	void rect(const glm::vec2 &pos, const glm::vec2 &size, const Texture2D &texture, Color tint);

	void square(const glm::vec2 &pos, float size, const Texture2D &texture);
	void square(const glm::vec2 &pos, float size, Color color);
	void square(const glm::vec2 &pos, float size, const Texture2D &texture, Color tint);

	void ellipse(const glm::vec2 &center, const glm::vec2 &size, Color color);
	void ellipse(const glm::vec2 &center, const glm::vec2 &size, const Texture2D &texture);
	void ellipse(const glm::vec2 &center, const glm::vec2 &size, const Texture2D &texture, Color tint);

	void circle(const glm::vec2 &center, float radius, Color color);
	void circle(const glm::vec2 &center, float radius, const Texture2D &texture);
	void circle(const glm::vec2 &center, float radius, const Texture2D &texture, Color color);

	void tri(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3, Color tint);

	void flush();

	void set_camera(const Camera &camera);
	void set_view_proj(const glm::mat4 &viewProj);

	void reset_stats();
	RenderStats get_stats();

}
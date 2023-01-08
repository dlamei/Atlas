#pragma once

#include <glm/glm.hpp>

#include "atl_types.h"
#include "camera.h"

namespace Atlas::Render2D {

	struct Vertex {
		glm::vec3 pos;
		glm::vec2 uv;
		glm::vec4 color;
	};

	void init();

	void rect(const glm::vec2 &pos, const glm::vec2 &size, Color color);
	void flush();

	void set_camera(const Camera &camera);
}
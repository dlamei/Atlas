#pragma once

#include "atl_types.h"

#include "camera.h"

namespace Atlas::Render2D {

	struct Vertex {
		glm::vec3 pos;
		glm::vec2 uv;
	};

	void init();


	void test_draw();
}
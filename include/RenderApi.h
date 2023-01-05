#pragma once

#include "atl_types.h"

namespace Atlas {

	namespace RenderApi {

		void frame_start();
		void frame_end();

		void enable_clear_color(bool b);
		void enable_clear_depth(bool b);
		void clear_color(Atlas::Color c);

		void begin(const Atlas::Texture2D &color);
		void begin(const Atlas::Texture2D &color, const Texture2D &depth);
		void begin(const Atlas::Framebuffer &frameBuffer);
		void end();

		void draw_indexed();

		void init();
		void resize_viewport(uint32_t width, uint32_t height);
	}


}
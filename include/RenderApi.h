#pragma once

#include "atl_types.h"

namespace Atlas {

	namespace Barrier {
		enum _ : uint32_t {
			ALL = 1 << 0,
			IMAGE_ACCESS = 1 << 1,
		};
	}
	using BarrierBits = uint32_t;

	void memory_barrier(BarrierBits barriers);

	namespace Render {

		void frame_start();
		void frame_end();

		void enable_clear_color(bool b);
		void enable_clear_depth(bool b);
		void clear_color(Atlas::RGBA c);

		void begin(const Atlas::Texture2D &color);
		void begin(const Atlas::Texture2D &color, const Texture2D &depth);
		void begin(const Atlas::Framebuffer &frameBuffer);
		void end();

		void draw_indexed(size_t size);
		void draw_instanced(uint32_t count, uint32_t first = 0);
		void flush();

		void init();
		void resize_viewport(uint32_t width, uint32_t height);
	}
}
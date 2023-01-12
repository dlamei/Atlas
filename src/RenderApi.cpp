#include "RenderApi.h"

#include "gl_utils.h"

namespace Atlas {

	namespace Render {

		struct CachedFramebuffer {
			Framebuffer framebuffer;
			bool used{ false };
		};

		struct RenderContext {
			std::unordered_map<size_t, CachedFramebuffer> framebuffers;
			bool clearColorBuffer{ true };
			bool clearDepthBuffer{ true };
			glm::vec4 clearColor{ 0, 0, 0, 0 };
		};

		static RenderContext s_GlobalRenderContext{};

		size_t framebuffer_create_hash(const FramebufferCreateInfo &info) {
			size_t hash = 0;

			for (const auto &col : info.colorAttachments) {
				if (col.is_init()) {
					hash ^= col.hash();
				}
			}

			if (info.depthAttachments.is_init()) hash ^= info.depthAttachments.hash();

			return hash;
		}

		Framebuffer get_framebuffer(const FramebufferCreateInfo &info) {

			size_t hash = framebuffer_create_hash(info);

			if (s_GlobalRenderContext.framebuffers.find(hash) != s_GlobalRenderContext.framebuffers.end()) {
				auto it = s_GlobalRenderContext.framebuffers.find(hash);
				it->second.used = true;
				return it->second.framebuffer;
			}

			Framebuffer fb{ info };
			CachedFramebuffer cFb{};
			cFb.framebuffer = fb;
			cFb.used = true;

			s_GlobalRenderContext.framebuffers.insert({ hash, cFb });

			return fb;
		}

		void begin(const Texture2D &color) {

			FramebufferCreateInfo fbInfo{};
			fbInfo.colorAttachments = { color };

			Framebuffer fb = get_framebuffer(fbInfo);
			begin(fb);
		}

		void begin(const Texture2D &color, const Texture2D &depth)
		{
			FramebufferCreateInfo fbInfo{};
			fbInfo.colorAttachments = { color };
			fbInfo.depthAttachments = depth;

			Framebuffer fb = get_framebuffer(fbInfo);
			begin(fb);
		}

		void begin(const Framebuffer &frameBuffer)
		{
			glm::vec4 col = s_GlobalRenderContext.clearColor;

			Framebuffer::bind(frameBuffer);

			glClearColor(col.r, col.g, col.b, col.a);
			glClear(s_GlobalRenderContext.clearColorBuffer ? GL_COLOR_BUFFER_BIT : 0 || s_GlobalRenderContext.clearDepthBuffer ? GL_DEPTH_BUFFER_BIT : 0);
		}

		void frame_start()
		{
			for (auto &it : s_GlobalRenderContext.framebuffers) {
				it.second.used = false;
			}
		}

		void frame_end()
		{
			auto &framebuffers = s_GlobalRenderContext.framebuffers;

			//TODO: maybe only delete when not used for multiple frames
			for (auto it = framebuffers.begin(); it != framebuffers.end();) {
				if (!it->second.used) it = framebuffers.erase(it);
				else it++;
			}

		}

		void enable_clear_color(bool b)
		{
			s_GlobalRenderContext.clearColorBuffer = b;
		}

		void enable_clear_depth(bool b)
		{
			s_GlobalRenderContext.clearDepthBuffer = b;
		}

		void clear_color(Color c)
		{
			s_GlobalRenderContext.clearColor = c.normalized();
		}

		void end()
		{
			Framebuffer::unbind();
		}

		void draw_indexed(size_t size)
		{
			auto &indexBuffer = get_bound_index_buffer();
			if (!indexBuffer.is_init()) {
				CORE_WARN("Render::draw_indexed: no index buffer was bound");
				return;
			}

			auto &vertexBuffer = get_bound_vertex_buffer();
			if (!vertexBuffer.is_init()) {
				CORE_WARN("Render::draw_indexed: no vertex buffer was bound");
				return;
			}

			glDrawElements(GL_TRIANGLES, (int)size, GL_UNSIGNED_INT, 0);
		}

		void init()
		{
			gl_utils::init_opengl();
		}

		void resize_viewport(uint32_t width, uint32_t height)
		{
			gl_utils::resize_viewport(width, height);
		}
	}

}

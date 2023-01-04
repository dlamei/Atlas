#include "application.h"
#include "window.h"

#include <imgui.h>
#include "imgui_build.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

struct Vertex {
	glm::vec3 pos;
	glm::vec2 uv;
};

namespace Atlas {


	Application *Application::s_Instance = nullptr;

	Application::Application()
	{
		CORE_ASSERT(!s_Instance, "Application already created!");
		s_Instance = this;

		m_ViewportSize = { 1600, 900 };

		WindowCreateInfo info = { "Atlas Engine", 1600, 900 };
		m_Window = make_scope<Window>(info);
		m_Window->set_event_callback(BIND_EVENT_FN(Application::on_event));

		RenderApi::init();

		m_ImGuiLayer = make_ref<ImGuiLayer>();
		push_layer(m_ImGuiLayer);

		m_ColorBuffer = Texture2D::color((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_DepthBuffer = Texture2D::depth((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

		m_Texture = Texture2D::load("assets/images/uv_checker.png", TextureFilter::NEAREST).value();

		{
			VertexLayout layout = VertexLayout::from(&Vertex::pos, &Vertex::uv);
			m_Shader = Shader::load("assets/shaders/default.vert", "assets/shaders/default.frag", layout);
			m_Shader.set("tex", 0);

			Buffer cameraBuffer = Buffer::uniform(glm::ortho(-1, 1, -1, 1), BufferUsage::DYNAMIC_DRAW);
			m_Shader.set("CameraBuffer", cameraBuffer);
		}

		{
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

			m_VertexBuffer = Buffer::vertex(vertices, 4);
			m_IndexBuffer = Buffer::index(indices, 6);
		}

		//RenderApi::enable_clear_color(false);
		//RenderApi::enable_clear_depth(false);
		RenderApi::clear_color({ 60, 5, 45 });

		m_CameraController.set_position({ 0, 0, 3 });
	}

	Application::~Application()
	{
		for (uint32_t i = 0; i < m_LayerStack.size(); i++) {
			Ref<Layer> layer = m_LayerStack.back();
			layer->on_detach();
			m_LayerStack.pop_back();
		}

		m_Window->destroy();
	}

	void Application::run()
	{

		m_LastFrameTime = (float)m_Window->get_time();

		while (!m_Window->should_close()) {
			RenderApi::frame_start();

			m_Window->on_update();
			if (m_Window->is_minimized()) continue;

			ATL_FRAME("MainThread");
			update();

			RenderApi::frame_end();
		}
	}

	void Application::update()
	{
		float time = (float)m_Window->get_time();
		Timestep timestep = time - m_LastFrameTime;
		m_LastFrameTime = time;

		m_ImGuiLayer->begin();

		for (Event e : m_QueuedEvents) on_event(e);
		m_QueuedEvents.clear();

		m_CameraController.on_update(timestep);
		for (auto &layer : m_LayerStack) layer->on_update(timestep);
		for (auto &layer : m_LayerStack) layer->on_imgui();

		m_Shader.get_uniform_buffer("CameraBuffer")
			.set_data(m_CameraController.get_camera().get_view_projection());

		RenderApi::begin(m_ColorBuffer, m_DepthBuffer);

		Shader::bind(m_Shader);
		Texture2D::bind(m_Texture);

		Buffer::bind_vertex(m_VertexBuffer);
		Buffer::bind_index(m_IndexBuffer);

		RenderApi::draw_indexed(6);
		RenderApi::end();

		render_viewport();

		m_ImGuiLayer->end();
	}

	void Application::update_frame()
	{
		get_instance()->update();
	}

	void Application::render_viewport()
	{
		ATL_EVENT();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");
		ImGui::PopStyleVar();

		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		glm::vec2 viewportBounds[2]{};
		viewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		viewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };
		auto viewportSize = viewportBounds[1] - viewportBounds[0];

		ImGui::Image(m_ColorBuffer, { viewportSize.x, viewportSize.y });
		ImGui::End();

		ImGui::ShowDemoWindow();

		if (viewportSize.x != m_ViewportSize.x || viewportSize.y != m_ViewportSize.y) {
			Atlas::ViewportResizedEvent event = { (uint32_t)viewportSize.x, (uint32_t)viewportSize.y };
			Atlas::Event e(event);
			queue_event(e);
		}
	}

	Window &Application::get_window()
	{
		return *get_instance()->m_Window.get();
	}

	Application *Application::get_instance()
	{
		CORE_ASSERT(s_Instance, "Application is not initialized!");
		return s_Instance;
	}

	glm::vec2 Application::get_mouse()
	{
		auto pos = get_instance()->m_Window->get_mouse_pos();
		return { pos.first, pos.second };
	}

	bool Application::is_key_pressed(KeyCode key)
	{
		return get_instance()->m_Window->is_key_pressed(key);
	}

	bool Application::is_mouse_pressed(int button)
	{
		return get_instance()->m_Window->is_mouse_button_pressed(button);
	}

	void Application::push_layer(Ref<Layer> layer)
	{
		m_LayerStack.push_back(layer);
		layer->on_attach();
	}

	void Application::queue_event(Event event)
	{
		m_QueuedEvents.push_back(event);
	}

	glm::vec2 &Application::get_viewport_size()
	{
		return get_instance()->m_ViewportSize;
	}

	void Application::on_event(Event &event)
	{
		//if (!event.in_category(EventCategoryMouse)) CORE_TRACE("event: {}", event.to_string());

		EventDispatcher(event)
			.dispatch<WindowResizedEvent>(BIND_EVENT_FN(Application::on_window_resized))
			.dispatch<ViewportResizedEvent>(BIND_EVENT_FN(Application::on_viewport_resized));

		m_CameraController.on_event(event);

		for (auto &layer : m_LayerStack) {
			if (event.handled) break;

			layer->on_event(event);
		}
	}

	bool Application::on_window_resized(WindowResizedEvent &e)
	{
		if (e.width == 0 || e.height == 0) m_WindowMinimized = true;
		else m_WindowMinimized = false;

		return false;
	}

	bool Application::on_viewport_resized(ViewportResizedEvent &e)
	{
		m_ViewportSize = { e.width, e.height };

		RenderApi::resize_viewport(e.width, e.height);

		if (e.width == 0 || e.height == 0) return false;

		m_ColorBuffer = Texture2D::color(e.width, e.height, TextureFilter::NEAREST);
		m_DepthBuffer = Texture2D::depth(e.width, e.height, TextureFilter::NEAREST);

		return false;
	}
}
#include "application.h"
#include "window.h"

#include <imgui.h>
#include "imgui_build.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "RenderApi.h"

#include "Render2D.h"

static const std::vector<uint32_t> s_Logo = {
#include "logo.embed"
};

struct Vertex {
	glm::vec3 pos;
	glm::vec2 uv;
};

namespace Atlas {

	Application *Application::s_Instance = nullptr;

	Application::Application(const ApplicationCreateInfo &info)
	{
		CORE_ASSERT(!s_Instance, "Application already created!");
		s_Instance = this;

		m_ViewportSize = { info.width, info.height };

		WindowCreateInfo winInfo;
		winInfo.title = info.title;
		winInfo.width = info.width;
		winInfo.height = info.height;
		winInfo.icon = s_Logo;

		m_Window = make_scope<Window>(winInfo);
		m_Window->set_event_callback(BIND_EVENT_FN(Application::on_event));

		Random::init();
		Render::init();

		m_ImGuiLayer = make_ref<ImGuiLayer>();
		push_layer(m_ImGuiLayer);

		m_ColorBuffer = Texture2D::rgba((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		//m_DepthBuffer = Texture2D::depth((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
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
			ATL_FRAME("MainThread");


			m_Window->on_update();
			if (m_Window->is_minimized()) continue;

			update();
		}
	}

	void Application::update()
	{
		ATL_EVENT();
		Render::frame_start();
		m_ImGuiLayer->begin();

		float time = (float)m_Window->get_time();
		Timestep timestep = time - m_LastFrameTime;
		m_LastFrameTime = time;

		for (Event e : m_QueuedEvents) on_event(e);
		m_QueuedEvents.clear();

		for (auto &layer : m_LayerStack) {
			layer->on_imgui();
			layer->on_update(timestep);
		}
		//for (auto &layer : m_LayerStack) layer->on_imgui();
		//for (auto &layer : m_LayerStack) layer->on_update(timestep);

		render_viewport();
		m_ImGuiLayer->end();
		Render::frame_end();
	}

	void Application::update_frame()
	{
		get_instance()->update();
	}

	void Application::render_viewport()
	{
		ATL_EVENT();

		ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");
		ImGui::PopStyleVar();

		ImGui::BeginChild("Viewport");

		auto viewportSize = ImGui::GetWindowSize();

		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0, 0, 0, 0 });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0, 0, 0, 0 });
		ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
		ImGui::ImageButton(m_ColorBuffer, { viewportSize.x, viewportSize.y }, { 0, 1 }, { 1, 0 }, 0);
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		m_ViewportFocus = ImGui::IsItemFocused();
		m_ViewportHovered = ImGui::IsItemHovered();

		ImVec2 windowPosition = ImGui::GetWindowPos();
		auto [windowRelMousePosX, windowRelMousePosY] = m_Window->get_mouse_pos();
		auto [windowPosX, windowPosY] = m_Window->get_window_pos();

		ImVec2 mousePositionAbsolute = { windowRelMousePosX + windowPosX, windowRelMousePosY + windowPosY };
		ImVec2 screenPositionAbsolute = ImGui::GetItemRectMin();
		ImVec2 mouseRel = mousePositionAbsolute - screenPositionAbsolute;
		m_ViewportMousePos = { mouseRel.x, mouseRel.y };

		ImGui::EndChild();
		ImGui::End();

		if (viewportSize.x != m_ViewportSize.x || viewportSize.y != m_ViewportSize.y) {
			Atlas::ViewportResizedEvent event = { (uint32_t)viewportSize.x, (uint32_t)viewportSize.y };
			Atlas::Event e(event);
			on_event(e);
			//queue_event(e);
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
		return get_instance()->m_ViewportMousePos;
	}

	glm::vec2 Application::get_window_pos()
	{
		auto pos = get_instance()->m_Window->get_window_pos();
		return { pos.first, pos.second };
	}

	bool Application::is_key_pressed(KeyCode key)
	{
		if (!is_viewport_focused()) return false;
		return get_instance()->m_Window->is_key_pressed(key);
	}

	bool Application::is_mouse_pressed(int button)
	{
		if (!is_viewport_focused()) return false;
		return get_instance()->m_Window->is_mouse_button_pressed(button);
	}

	bool Application::is_viewport_focused()
	{
		return get_instance()->m_ViewportFocus;
	}

	bool Application::is_viewport_hovered()
	{
		return get_instance()->m_ViewportHovered;
	}

	float Application::get_time()
	{
		return (float)get_instance()->m_Window->get_time();
	}

	Texture2D &Application::get_viewport_color()
	{
		return get_instance()->m_ColorBuffer;
	}

	//Texture2D &Application::get_viewport_depth()
	//{
	//	return get_instance()->m_DepthBuffer;
	//}

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
		if (event.in_category(EventCategory::Input) && !m_ViewportHovered) return;


		EventDispatcher(event)
			.dispatch<MouseMovedEvent>(BIND_EVENT_FN(Application::on_mouse_moved))
			.dispatch<WindowResizedEvent>(BIND_EVENT_FN(Application::on_window_resized))
			.dispatch<ViewportResizedEvent>(BIND_EVENT_FN(Application::on_viewport_resized));

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

	void Application::set_vsync(bool enable)
	{
		get_instance()->m_Window->set_vsync(enable);
	}

	bool Application::on_viewport_resized(ViewportResizedEvent &e)
	{
		m_ViewportSize = { e.width, e.height };

		Render::resize_viewport(e.width, e.height);

		if (e.width == 0 || e.height == 0) return false;

		m_ColorBuffer = Texture2D::rgba(e.width, e.height, TextureFilter::NEAREST);
		//m_DepthBuffer = Texture2D::depth(e.width, e.height, TextureFilter::NEAREST);

		return false;
	}

	bool Application::on_mouse_moved(MouseMovedEvent &e)
	{
		e.mouseX = m_ViewportMousePos.x;
		e.mouseY = m_ViewportMousePos.y;
		return false;
	}
}
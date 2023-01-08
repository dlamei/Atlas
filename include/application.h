#pragma once
#include "event.h"
#include "atl_types.h"

#include "camera.h"

namespace Atlas {
	class Window;

	class ImGuiLayer;

	class Timestep
	{
	private:
		float m_Time;

	public:
		Timestep(float time = 0.0f)
			: m_Time(time) {}

		operator float() const { return m_Time; }

		inline float GetSeconds() const { return m_Time; }
		inline float GetMilliseconds() const { return m_Time * 1000.0f; }
	};

	class Layer
	{
	public:
		virtual ~Layer() {};

		virtual void on_attach() {}
		virtual void on_detach() {}
		virtual void on_update(Timestep ts) {}
		virtual void on_imgui() {}
		virtual void on_event(Event &event) {}
	};

	class Application {
	public:

		Application();
		~Application();

		void run();
		void update();

		static void update_frame();

		static Window &get_window();
		static Application *get_instance();
		static glm::vec2 get_mouse();
		static glm::vec2 get_window_pos();
		static bool is_key_pressed(KeyCode key);
		static bool is_mouse_pressed(int button);
		static bool is_viewport_focused();
		static bool is_viewport_hovered();
		static Texture2D &get_viewport_color();
		static Texture2D &get_viewport_depth();

		void push_layer(Ref<Layer> layer);

		void queue_event(Event event);

		static glm::vec2 &get_viewport_size();

	private:
		void on_event(Event &event);
		bool on_window_resized(WindowResizedEvent &e);
		bool on_viewport_resized(ViewportResizedEvent &e);
		bool on_mouse_moved(MouseMovedEvent &e);

		void render_viewport();

		Scope<Window> m_Window;
		bool m_WindowMinimized = false;

		float m_LastFrameTime{ 0 };

		Ref<ImGuiLayer> m_ImGuiLayer;
		std::vector<Ref<Layer>> m_LayerStack;

		std::vector<Event> m_QueuedEvents;

		Texture2D m_ColorBuffer;
		Texture2D m_DepthBuffer;

		glm::vec2 m_ViewportSize;
		bool m_ViewportFocus;
		bool m_ViewportHovered;
		glm::vec2 m_ViewportMousePos;

		static Application *s_Instance;
	};

}

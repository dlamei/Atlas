#pragma once
#include "event.h"

#include <glm/glm.hpp>


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

		static Window &get_window();
		static Application *get_instance();
		static glm::vec2 get_mouse();
		static bool is_key_pressed(KeyCode key);
		static bool is_mouse_pressed(int button);

		void push_layer(Ref<Layer> layer);

		void queue_event(Event event);

		static glm::vec2 &get_viewport_size();

		//static Ref<Texture> get_viewport_color_texture();
		//static Ref<Texture> get_viewport_depth_texture();

	private:
		void on_event(Event &event);
		bool on_window_resized(WindowResizedEvent &e);
		bool on_viewport_resized(ViewportResizedEvent &e);

		void render_viewport();

		Scope<Window> m_Window;
		bool m_WindowMinimized = false;

		float m_LastFrameTime{ 0 };

		Ref<ImGuiLayer> m_ImGuiLayer;
		std::vector<Ref<Layer>> m_LayerStack;

		std::vector<Event> m_QueuedEvents;

		//Ref<Texture> m_ColorTexture;
		//Ref<Texture> m_DepthTexture;
		glm::vec2 m_ViewportSize;

		//float data[100] = { 1, 2, 3, 4, 3, 2, 1 };

		static Application *s_Instance;
	};

}

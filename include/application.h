#pragma once
#include "event.h"
#include "layer.h"

#include <glm/glm.hpp>


namespace Atlas {
	class Window;

	class Application {
	public:

		Application();
		~Application();

		void run();

		//static vkutil::VulkanManager &get_vulkan_manager();
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

		//Ref<ImGuiLayer> m_ImGuiLayer;
		std::vector<Ref<Layer>> m_LayerStack;

		std::vector<Event> m_QueuedEvents;

		//Ref<Texture> m_ColorTexture;
		//Ref<Texture> m_DepthTexture;
		glm::vec2 m_ViewportSize;

		//float data[100] = { 1, 2, 3, 4, 3, 2, 1 };

		static Application *s_Instance;
	};

}

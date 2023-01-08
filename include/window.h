#pragma once
#include "event.h"

struct GLFWwindow;

namespace Atlas {

	using EventCallbackFn = std::function<void(Event &)>;

	void default_event_callback_fn(Event &e);

	struct WindowCreateInfo {
		std::string title{ "Atlas" };
		uint32_t width{ 0 }, height{ 0 };

		EventCallbackFn eventCallback = default_event_callback_fn;
	};

	class Window {

	public:
		Window(const WindowCreateInfo &info);
		void destroy();

		void on_update();

		inline uint32_t get_width() const { return m_Width; }
		inline uint32_t get_height() const { return m_Height; }

		double get_time();
		bool should_close() const;

		void capture_mouse(bool enabled);

		bool is_key_pressed(Atlas::KeyCode key);
		bool is_mouse_button_pressed(int button);
		bool is_minimized();

		void set_event_callback(const EventCallbackFn &callback);
		const EventCallbackFn &get_event_callback() const;

		std::pair<float, float> get_mouse_pos() const;
		std::pair<float, float> get_window_pos() const;
		inline GLFWwindow *get_native_window() const { return m_Window; }

	private:
		void init();

		GLFWwindow *m_Window;

		std::string m_Title;
		uint32_t m_Width{ 0 };
		uint32_t m_Height{ 0 };

		bool m_CaptureMouse{ false };

		EventCallbackFn m_EventCallBackFn;
	};

} // namespace Atlas

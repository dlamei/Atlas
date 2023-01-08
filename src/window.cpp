#include "window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "application.h"

namespace Atlas {

	void default_event_callback_fn(Event &e) {}

	static bool s_GLFWInitialized = false;

	static void GLFWErrorCallback(int error, const char *message) {
		CORE_ERROR("GLFW Error ({0}): {1}", error, message);
	}

	Window::Window(const WindowCreateInfo &info)
		: m_Title(info.title), m_Width(info.width), m_Height(info.height),
		m_EventCallBackFn(info.eventCallback) {
		init();
	}

	inline const EventCallbackFn &Window::get_event_callback() const {
		return m_EventCallBackFn;
	}

	bool Window::is_key_pressed(KeyCode key) {
		auto state = glfwGetKey(m_Window, (int)key);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Window::is_mouse_button_pressed(int button) {
		auto state = glfwGetMouseButton(m_Window, button);
		return state == GLFW_PRESS;
	}

	bool Window::is_minimized()
	{
		return !(m_Width && m_Height);
	}

	//std::pair<float, float> Window::get_mouse_pos() {
	//	double mouseX, mouseY;
	//	glfwGetCursorPos(m_Window, &mouseX, &mouseY);
	//	return { (float)mouseX, (float)mouseY };
	//}

	void Window::init() {

		CORE_TRACE("Creating window {0} ({1}, {2})", m_Title, m_Width, m_Height);

		if (!s_GLFWInitialized) {
			int success = glfwInit();
			CORE_ASSERT(success, "Could not intialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);

			s_GLFWInitialized = true;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); //TODO: enable
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_Window = glfwCreateWindow((int)m_Width, (int)m_Height, m_Title.c_str(),
			nullptr, nullptr);

		CORE_ASSERT(m_Window, "Could not create window");

		glfwMakeContextCurrent(m_Window);
		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		glfwSwapInterval(1);

		{
			int w, h;
			glfwGetFramebufferSize(m_Window, &w, &h);
			m_Width = static_cast<uint32_t>(w);
			m_Height = static_cast<uint32_t>(h);
		}

		glfwSetInputMode(m_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		glfwSetWindowUserPointer(m_Window, this);

		glfwSetWindowSizeCallback(
			m_Window, [](GLFWwindow *window, int width, int height) {
				ATL_EVENT("glfwWindowSizeCallback");
		Window &data = *(Window *)glfwGetWindowUserPointer(window);
		data.m_Width = width;
		data.m_Height = height;

		WindowResizedEvent event{ (uint32_t)width, (uint32_t)height };
		Event e(event);
		data.m_EventCallBackFn(e);

		Application::update_frame();
		glfwSwapBuffers(data.get_native_window());
			});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow *window) {
			Window &data = *(Window *)glfwGetWindowUserPointer(window);
		WindowClosedEvent event;
		Event e(event);
		data.m_EventCallBackFn(e);
			});

		glfwSetKeyCallback(m_Window, [](GLFWwindow *window, int key, int scancode,
			int action, int mods) {
				ATL_EVENT("glfwKeyCallback");
		Window &data = *(Window *)glfwGetWindowUserPointer(window);

		switch (action) {
		case GLFW_PRESS: {
			KeyPressedEvent event{ key, 0 };
			Event e(event);
			data.m_EventCallBackFn(e);
			break;
		}
		case GLFW_RELEASE: {
			KeyReleasedEvent event{ key };
			Event e(event);
			data.m_EventCallBackFn(e);
			break;
		}
		case GLFW_REPEAT: {
			KeyPressedEvent event{ key, true };
			Event e(event);
			data.m_EventCallBackFn(e);
			break;
		}
		}
			});

		glfwSetCharCallback(m_Window, [](GLFWwindow *window, unsigned int keyCode) {
			ATL_EVENT("glfwCharCallback");
		Window &data = *(Window *)glfwGetWindowUserPointer(window);
		KeyTypedEvent event{ (char)keyCode };
		Event e(event);
		data.m_EventCallBackFn(e);
			});

		glfwSetMouseButtonCallback(
			m_Window, [](GLFWwindow *window, int button, int action, int mods) {
				ATL_EVENT("glfwMouseButtonCallback");
		Window &data = *(Window *)glfwGetWindowUserPointer(window);

		switch (action) {
		case GLFW_PRESS: {
			MouseButtonPressedEvent event{ button };
			Event e(event);
			data.m_EventCallBackFn(e);
			break;
		}
		case GLFW_RELEASE: {
			MouseButtonReleasedEvent event{ button };
			Event e(event);
			data.m_EventCallBackFn(e);
			break;
		}
		}
			});

		glfwSetScrollCallback(
			m_Window, [](GLFWwindow *window, double xOffset, double yOffset) {
				ATL_EVENT("glfwScrollCallback");
		Window &data = *(Window *)glfwGetWindowUserPointer(window);

		MouseScrolledEvent event{ (float)xOffset, (float)yOffset };
		Event e(event);
		data.m_EventCallBackFn(e);
			});

		glfwSetCursorPosCallback(
			m_Window, [](GLFWwindow *window, double xPos, double yPos) {
				ATL_EVENT("glfwCursorCallback");
		Window &data = *(Window *)glfwGetWindowUserPointer(window);

		MouseMovedEvent event{ (float)xPos, (float)yPos };
		Event e(event);
		data.m_EventCallBackFn(e);
			});
	}

	void Window::destroy() { glfwDestroyWindow(m_Window); }

	void Window::on_update() {
		ATL_EVENT();
		glfwPollEvents();
		glfwSwapBuffers(m_Window);
	}

	std::pair<float, float> Window::get_mouse_pos() const {
		double mouseX, mouseY;
		glfwGetCursorPos(m_Window, &mouseX, &mouseY);
		return { (float)mouseX, (float)mouseY };
	}

	std::pair<float, float> Window::get_window_pos() const
	{
		int posX, posY;
		glfwGetWindowPos(m_Window, &posX, &posY);
		return { (float)posX, (float)posY };
	}

	double Window::get_time()
	{
		return glfwGetTime();
	}

	bool Window::should_close() const { return glfwWindowShouldClose(m_Window); }

	void Window::set_event_callback(const EventCallbackFn &callback) {
		m_EventCallBackFn = callback;
	}

	void Window::capture_mouse(bool enabled) {
		if (enabled == m_CaptureMouse) return;

		if (enabled) {
			glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			m_CaptureMouse = true;
		}
		else {
			glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			m_CaptureMouse = false;
		}
	}

} // namespace Atlas

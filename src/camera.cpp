#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "application.h"
#include "window.h"

namespace Atlas {

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		: m_ProjectionMatrix(glm::ortho(left, right, bottom, top, m_Near, m_Far)), m_ViewMatrix(1.0f)
	{
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top, float near, float far)
		: m_Near(near), m_Far(far), m_ProjectionMatrix(glm::ortho(left, right, bottom, top, near, far)), m_ViewMatrix(1.0f)
	{
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::set_projection(float left, float right, float bottom, float top)
	{
		m_ProjectionMatrix = glm::ortho(left, right, bottom, top, m_Near, m_Far);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::recalculate_view()
	{
		glm::mat4 transform = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1)) * glm::translate(glm::mat4(1.0f), m_Position);

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;

	}

	bool OrthographicCameraController::on_mouse_scrolled(MouseScrolledEvent &e)
	{
		m_ZoomLevel -= e.offsetY * 0.15f * m_ZoomLevel;
		m_Camera.set_projection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}

	OrthographicCameraController::OrthographicCameraController(bool rotation)
		: m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio *m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel), m_Rotation(rotation) {
		auto size = Application::get_viewport_size();
		m_AspectRatio = (float)size.x / (float)size.y;
	}

	void OrthographicCameraController::on_update(float timestep)
	{
		//if (Input::IsKeyPressed(ATL_KEY_D))
		if (Application::is_key_pressed(KeyCode::D))
		{
			m_CameraPosition.x += m_CameraTranslationSpeed * timestep;
		}
		else if (Application::is_key_pressed(KeyCode::A))
		{
			m_CameraPosition.x -= m_CameraTranslationSpeed * timestep;
		}

		if (Application::is_key_pressed(KeyCode::S))
		{
			m_CameraPosition.y -= m_CameraTranslationSpeed * timestep;
		}
		else if (Application::is_key_pressed(KeyCode::W))
		{
			m_CameraPosition.y += m_CameraTranslationSpeed * timestep;
		}

		if (m_Rotation)
		{
			if (Application::is_key_pressed(KeyCode::Q))
			{
				m_CameraRotation -= m_CameraRotationSpeed * timestep;
			}
			else if (Application::is_key_pressed(KeyCode::E))
			{
				m_CameraRotation += m_CameraRotationSpeed * timestep;
			}

			if (m_CameraRotation > 180.0f)
			{
				m_CameraRotation -= 360.0f;
			}
			else if (m_CameraRotation <= -180.0f)
			{
				m_CameraRotation += 360.0f;
			}

			m_Camera.set_rotation(m_CameraRotation);
		}

		m_Camera.set_position(m_CameraPosition);

		m_CameraTranslationSpeed = m_ZoomLevel * 2;

		glm::vec2 &viewportSize = Application::get_viewport_size();

		float aspecRatio = (float)viewportSize.x / (float)viewportSize.y;
		if (aspecRatio != m_AspectRatio)
		{
			m_AspectRatio = aspecRatio;
			m_Camera.set_projection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		}
	}

	void OrthographicCameraController::on_event(Event &e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<MouseScrolledEvent>(BIND_EVENT_FN(OrthographicCameraController::on_mouse_scrolled));
	}

	void PerspectiveCamera::recalculate_view()
	{
		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void PerspectiveCamera::recalculate_projection()
	{
		m_ProjectionMatrix = glm::perspective(m_Fov, m_AspectRatio, m_NearPlane, m_FarPlane);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	PerspectiveCamera::PerspectiveCamera(float nearPlane, float farPlane, float fov, float aspectRatio)
		: m_NearPlane(nearPlane), m_FarPlane(farPlane), m_Fov(fov), m_AspectRatio(aspectRatio),
		m_ProjectionMatrix(glm::perspective(fov, aspectRatio, nearPlane, farPlane))
	{
		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void PerspectiveCamera::set_front(const glm::vec3 &direction)
	{
		m_Front = direction;
		m_Right = glm::normalize(glm::cross(m_Front, m_Up));
		recalculate_view();
	}

	void PerspectiveCamera::set_projection(float nearPlane, float farPlane, float fov, float aspectRatio)
	{
		m_NearPlane = nearPlane;
		m_FarPlane = farPlane;
		m_Fov = fov;
		m_AspectRatio = aspectRatio;
		m_ProjectionMatrix = glm::perspective(m_Fov, m_AspectRatio, m_NearPlane, m_FarPlane);
	}

	bool PerspectiveCameraController::on_mouse_moved(MouseMovedEvent &e)
	{
		if (!Application::is_mouse_pressed(1)) return false;

		if (firstMouseMove)
		{
			m_PMouseX = e.mouseX;
			m_PMouseY = e.mouseY;
			firstMouseMove = false;
		}

		float xOffset = e.mouseX - m_PMouseX;
		float yOffset = m_PMouseY - e.mouseY;
		m_PMouseX = e.mouseX;
		m_PMouseY = e.mouseY;

		m_Yaw += xOffset * m_CamearSensitivity;
		m_Pitch += yOffset * m_CamearSensitivity;

		m_Pitch = std::clamp(m_Pitch, -89.0f, 89.0f);

		m_CameraDirection.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		m_CameraDirection.y = sin(glm::radians(m_Pitch));
		m_CameraDirection.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

		m_Camera.set_front(glm::normalize(m_CameraDirection));

		return false;
	}

	bool PerspectiveCameraController::on_mouse_pressed(MouseButtonPressedEvent &e)
	{
		Window &window = Application::get_window();
		if (e.button == 1) window.capture_mouse(true);
		return false;
	}

	bool PerspectiveCameraController::on_mouse_released(MouseButtonReleasedEvent &e)
	{
		Window &window = Application::get_window();
		window.capture_mouse(false);
		firstMouseMove = true;
		return false;
	}

	bool PerspectiveCameraController::on_window_resized(WindowResizedEvent &e)
	{
		m_Camera.set_aspect_ratio((float)e.width / (float)e.height);
		return false;
	}

	PerspectiveCameraController::PerspectiveCameraController(float aspecRatio)
		: m_Camera(0.1f, 1000.0f, glm::radians(90.0f), aspecRatio) {}

	void PerspectiveCameraController::on_update(float ts)
	{
		bool updated = false;

		if (Application::is_key_pressed(KeyCode::LEFT_SHIFT))
		{
			m_CameraMoveSpeed = 50.0f;
		}
		else {
			m_CameraMoveSpeed = 5.0f;
		}

		if (Application::is_key_pressed(KeyCode::S))
		{
			m_CameraPosition -= m_Camera.get_front() * (m_CameraMoveSpeed * ts);
			updated = true;
		}
		if (Application::is_key_pressed(KeyCode::W))
		{
			m_CameraPosition += m_Camera.get_front() * (m_CameraMoveSpeed * ts);
			updated = true;
		}
		if (Application::is_key_pressed(KeyCode::A))
		{
			m_CameraPosition -= m_Camera.get_right() * (m_CameraMoveSpeed * ts);
			updated = true;
		}
		if (Application::is_key_pressed(KeyCode::D))
		{
			m_CameraPosition += m_Camera.get_right() * (m_CameraMoveSpeed * ts);
			updated = true;
		}


		if (Application::is_key_pressed(KeyCode::LEFT_CONTROL))
		{
			m_CameraPosition.y -= m_CameraMoveSpeed * ts;
			updated = true;
		}
		if (Application::is_key_pressed(KeyCode::SPACE))
		{
			m_CameraPosition.y += m_CameraMoveSpeed * ts;
			updated = true;
		}

		if (updated) m_Camera.set_position(m_CameraPosition);

		glm::vec2 &viewportSize = Application::get_viewport_size();
		float aspecRatio = viewportSize.x / viewportSize.y;
		if (aspecRatio != m_Camera.get_aspect_ratio())
		{
			m_Camera.set_aspect_ratio(aspecRatio);
		}

	}

	void PerspectiveCameraController::on_event(Event &e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<MouseMovedEvent>(BIND_EVENT_FN(PerspectiveCameraController::on_mouse_moved))
			.dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(PerspectiveCameraController::on_mouse_pressed))
			.dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN(PerspectiveCameraController::on_mouse_released));
	}

}

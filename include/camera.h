#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "event.h"

namespace Atlas {

	class Camera {
	public:

		virtual void set_position(const glm::vec3 &pos) = 0;

		virtual const glm::vec3 &get_position() const = 0;
		virtual const glm::mat4 &get_projection() const = 0;
		virtual const glm::mat4 &get_view() const = 0;
		virtual const glm::mat4 &get_view_projection() const = 0;

		virtual ~Camera() {}
	};

	class CameraController {
	public:
		virtual void on_update(float ts) = 0;
		virtual void on_event(Event &e) = 0;
		virtual void set_position(const glm::vec3 &pos) = 0;
		virtual const glm::vec3 &get_position() = 0;

		virtual Camera &get_camera() = 0;

	};

	class OrthographicCamera : public Camera
	{
	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		float m_Rotation = 0.0f;

		float m_Near = -20.0f;
		float m_Far = 20.0;

		void recalculate_view();

	public:
		OrthographicCamera(float left, float right, float bottom, float top);
		OrthographicCamera(float left, float right, float bottom, float top, float near, float far);

		const glm::vec3 &get_position() const override { return m_Position; }
		void set_position(const glm::vec3 &position) override { m_Position = position; recalculate_view(); }

		inline const float get_rotation() const { return m_Rotation; }
		void set_rotation(float rotation) { m_Rotation = rotation; recalculate_view(); }

		void set_projection(float left, float right, float bottom, float top);

		const glm::mat4 &get_projection() const override { return m_ProjectionMatrix; }
		const glm::mat4 &get_view() const override { return m_ViewMatrix; }
		const glm::mat4 &get_view_projection() const override { return m_ViewProjectionMatrix; }
	};

	class OrthographicCameraController : public CameraController
	{
	private:
		float m_AspectRatio = 0.5f;
		float m_ZoomLevel = 1.0f;

		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		bool m_Rotation;
		float m_CameraRotation = 0.0f;
		float m_CameraTranslationSpeed = 5.0f, m_CameraRotationSpeed = 180.0f;

		OrthographicCamera m_Camera;

		bool on_mouse_scrolled(MouseScrolledEvent &e);

	public:
		OrthographicCameraController(bool roation = false);

		void on_update(float ts) override;
		void on_event(Event &e) override;

		void set_position(const glm::vec3 &pos) override;
		void set_camera(float left, float right, float bottom, float top);

		const glm::vec3 &get_position() override;

		OrthographicCamera &get_camera() override { return m_Camera; }
	};

	class PerspectiveCamera : public Camera
	{
	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_Front = { 0.0f, 0.0f, -1.0f };
		glm::vec3 m_Up = { 0.0f, 1.0f, 0.0f };
		glm::vec3 m_Right = glm::normalize(glm::cross(m_Front, m_Up));
		glm::vec3 m_WorldUp = { 0.0f, 1.0f, 0.0f };

		float m_NearPlane, m_FarPlane, m_Fov, m_AspectRatio;

		void recalculate_view();
		void recalculate_projection();

	public:
		PerspectiveCamera(float nearPlane, float farPlane, float fov, float aspectRatio);

		void set_position(const glm::vec3 &position) override { m_Position = position; recalculate_view(); }
		void set_front(const glm::vec3 &direction);
		void set_projection(float nearPlane, float farPlane, float fov, float aspectRatio);
		void set_fov(float fov) { m_Fov = fov; recalculate_projection(); }
		void set_aspect_ratio(float aspectRatio) { m_AspectRatio = aspectRatio; recalculate_projection(); }

		const glm::mat4 &get_projection() const override { return m_ProjectionMatrix; }
		const glm::mat4 &get_view() const override { return m_ViewMatrix; }
		const glm::mat4 &get_view_projection() const override { return m_ViewProjectionMatrix; }
		const glm::vec3 &get_up() { return m_Up; }
		const glm::vec3 &get_front() { return m_Front; }
		const glm::vec3 &get_position() const override { return m_Position; }
		const glm::vec3 &get_right() { return m_Right; }

		inline float get_aspect_ratio() { return m_AspectRatio; }

	};

	class PerspectiveCameraController : public CameraController
	{
	private:
		float m_CameraMoveSpeed = 5.0f, m_CamearSensitivity = 0.2f;
		float m_Yaw = -90.0f, m_Pitch = 0.0f;

		float m_PMouseX = 0.0f, m_PMouseY = 0.0f;
		bool m_FirstMouseMove = true;

		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_CameraDirection = { 0.0f, 0.0f, 0.0f };

		PerspectiveCamera m_Camera;

		bool on_mouse_moved(MouseMovedEvent &e);
		bool on_mouse_pressed(MouseButtonPressedEvent &e);
		bool on_mouse_released(MouseButtonReleasedEvent &e);
		bool on_window_resized(WindowResizedEvent &e);

	public:
		PerspectiveCameraController(float aspecRatio = 1.5707f);

		void on_update(float ts) override;
		void on_event(Event &e) override;
		void set_position(const glm::vec3 &pos) override;
		const glm::vec3 &get_position() override;

		inline void set_fov(float fov) { m_Camera.set_fov(fov); }
		inline void set_aspect_ratio(float aspectRatio) { m_Camera.set_aspect_ratio(aspectRatio); }

		inline float get_aspect_ratio() { return m_Camera.get_aspect_ratio(); }

		PerspectiveCamera &get_camera() override { return m_Camera; }

		const glm::mat4 &get_view() { return m_Camera.get_view(); }
		const glm::mat4 &get_projection() { return m_Camera.get_projection(); }
	};


}

#pragma once

#include "event.h"

namespace Atlas
{
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

}


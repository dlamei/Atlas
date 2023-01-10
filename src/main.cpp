#include <iostream>

#include "application.h"
#include "Render2D.h"
#include "camera.h"
#include "atl_types.h"
#include "RenderApi.h"

class Sandbox : public Atlas::Layer {

	const uint32_t size = 200;

	Atlas::OrthographicCameraController controller;
	Atlas::Texture2D texture;
	glm::ivec2 offset{};

	void on_attach() override {
		Atlas::Render2D::init();
		controller.set_camera(0, (float)size, 0, (float)size);
		texture = Atlas::Texture2D::load("assets/images/uv_checker.png", Atlas::TextureFilter::NEAREST).value();
	}

	void on_detach() override {
	}

	void on_update(Atlas::Timestep ts) override {
		using namespace Atlas;

		controller.on_update(ts);

		Render2D::set_camera(controller.get_camera());

		ImGui::ShowDemoWindow();

		Render::begin(Application::get_viewport_color());

		for (uint32_t i = 0; i < size; i++) {
			for (uint32_t j = 0; j < size; j++) {
				glm::vec3 color{};
				uint32_t x = (i + offset.x) % size;
				uint32_t y = (j + offset.y) % size;
				color.r = (float)x / size;
				color.g = (float)y / size;
				color.b = 1 - ((float)i + j) / (size * 2);
				Render2D::rect({ i, j }, { 1, 1 }, Color::from_normalized(color));
			}
		}

		Render2D::flush();
		Render::end();
	}

	void on_imgui() override {
		ImGui::Begin("Settings");
		ImGui::DragInt2("offset", &offset[0], 1.0f, 0, size);
		ImGui::End();
	}

	void on_event(Atlas::Event &e) override {
		controller.on_event(e);
	}

};

int main() {

	Atlas::Application app;
	app.push_layer(make_ref<Sandbox>());
	app.run();
}

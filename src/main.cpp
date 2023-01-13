#include <iostream>

#include "application.h"
#include "Render2D.h"
#include "camera.h"
#include "atl_types.h"
#include "RenderApi.h"

class Sandbox : public Atlas::Layer {

	int32_t size = 1;
	float triSize = 0.25;
	bool useCircles{ false };

	Atlas::OrthographicCameraController controller;
	Atlas::Texture2D texture;

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


		Render::begin(Application::get_viewport_color());

		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				glm::vec3 color{};
				color.r = (float)i / size;
				color.g = (float)j / size;
				color.b = 1 - ((float)i + j) / (size * 2);
				if (useCircles) {
					Render2D::circle({ i + 0.5, j + 0.5 }, 0.5, texture, Color::from_norm(color));
				}
				else {
					Render2D::rect({ i, j }, { 1, 1 }, texture, Color::from_norm(color));
				}
			}
		}

		Render2D::tri({ size * triSize, size * triSize }, { size / 2.0f, size - size * triSize }, { size - size * triSize, size * triSize }, { 200, 0, 100, 100 });

		Render2D::flush();
		Render::end();
	}

	void on_imgui() override {
		ImGui::ShowDemoWindow();

		ImGui::Begin("Settings");
		ImGui::Checkbox("circles", &useCircles);
		int tempSize = size;
		ImGui::DragInt("size", &tempSize, .1f, 1, 200);
		if (tempSize != size) {
			size = tempSize;
			controller.set_camera(0, (float)size, 0, (float)size);
		}
		ImGui::DragFloat("Triangle Size", &triSize, .001f, 0, 1);
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

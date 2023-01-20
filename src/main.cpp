#include <iostream>

#include "application.h"
#include "Render2D.h"
#include "camera.h"
#include "atl_types.h"
#include "RenderApi.h"

class Sandbox : public Atlas::Layer {

	int32_t size = 1;

	Atlas::OrthographicCameraController controller;
	Atlas::Shader computeShader;
	Atlas::Texture2D compOut;

	void on_attach() override {
		Atlas::Render2D::init();
		controller.set_camera(0, (float)size, 0, (float)size);
		//texture = Atlas::Texture2D::load("assets/images/uv_checker.png", Atlas::TextureFilter::NEAREST).value();
		computeShader = Atlas::Shader::load_comp("assets/shaders/tex.comp");
		compOut = Atlas::Texture2D::rgba(1024, 1024);
	}

	void on_detach() override {
	}

	void on_update(Atlas::Timestep ts) override {
		using namespace Atlas;

		controller.on_update(ts);

		Render2D::set_camera(controller.get_camera());

		computeShader.bind("imgOutput", compOut, TextureUsage::READ | TextureUsage::WRITE);
		computeShader.set("width", (float)compOut.width());
		computeShader.set("time", Application::get_time());
		Shader::dispatch(computeShader, compOut.width() / 32, compOut.height() / 32, 1);

		memory_barrier(Barrier::IMAGE_ACCESS);

		Render::begin(Application::get_viewport_color());

		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				Render2D::rect({ i, j }, { 1, 1 }, compOut);
			}
		}

		Render2D::flush();

		Render::end();

	}

	void on_imgui() override {
		ImGui::Begin("output");
		ImGui::Image(compOut, { 255, 255 });
		ImGui::End();

		ImGui::Begin("Settings");
		int tempSize = size;
		ImGui::DragInt("size", &tempSize, .1f, 1, 200);
		if (tempSize != size) {
			size = tempSize;
			controller.set_camera(0, (float)size, 0, (float)size);
		}
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

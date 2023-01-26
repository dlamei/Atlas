#include <iostream>

#include "application.h"
#include "Render2D.h"
#include "camera.h"
#include "atl_types.h"
#include "RenderApi.h"

class Sandbox : public Atlas::Layer {

	Atlas::OrthographicCameraController controller;
	Atlas::Shader computeShader;
	Atlas::Texture2D compIn;
	Atlas::Texture2D compOut;

	void on_attach() override {
		using namespace Atlas;
		Render2D::init();

		controller.set_camera(0, 1, 0, 1);

		computeShader = Shader::load_comp("assets/shaders/game_of_life.comp");

		compIn = Texture2D::load("assets/images/uv_checker.png", TextureFilter::NEAREST).value();
		compOut = Texture2D::load("assets/images/uv_checker.png", TextureFilter::NEAREST).value();

	}

	void on_detach() override {
	}

	void on_update(Atlas::Timestep ts) override {
		using namespace Atlas;
		ATL_EVENT("layer update");

		controller.on_update(ts);
		Render2D::set_camera(controller.get_camera());

		computeShader.bind("inputBoard", compIn, TextureUsage::READ);
		computeShader.bind("outputBoard", compOut, TextureUsage::WRITE);
		Shader::dispatch(computeShader, compIn.width() / 32, compIn.height() / 32, 1);

		memory_barrier(Barrier::IMAGE_ACCESS);

		Render::begin(Application::get_viewport_color());
		Render2D::square({ 0, 0 }, 1, compOut);

		Render2D::flush();
		Render::end();

		std::swap(compIn, compOut);
	}

	void on_imgui() override {
		using namespace Atlas;

		ImGui::Begin("Settings");
		if (ImGui::Button("reload texture")) {
			compIn = Texture2D::load("assets/images/uv_checker.png", TextureFilter::NEAREST).value();
			compOut = Texture2D::load("assets/images/uv_checker.png", TextureFilter::NEAREST).value();
		}
		ImGui::End();
	}

	void on_event(Atlas::Event &e) override {
		controller.on_event(e);
	}

};

int main() {
	Atlas::Application app = Atlas::Application::default();
	app.push_layer(make_ref<Sandbox>());
	app.run();
}

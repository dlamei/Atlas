#include <iostream>

#include "application.h"
#include "Render2D.h"
#include "camera.h"
#include "atl_types.h"
#include "RenderApi.h"

void fill_random(Atlas::Texture2D &tex) {
	std::vector<Atlas::RGBA> arr(tex.width() * tex.height());

	for (uint32_t i = 0; i < arr.size(); i++) {
		arr.at(i) = Atlas::RGBA(Atlas::Random::get<uint8_t>());
	}

	tex.fill(arr.data(), arr.size() * sizeof(Atlas::RGBA));
}

class Sandbox : public Atlas::Layer {

	Atlas::OrthographicCameraController controller;
	Atlas::Shader computeShader;
	Atlas::Texture2D compIn;
	Atlas::Texture2D compOut;

	int size = 1024;

	void on_attach() override {
		using namespace Atlas;
		Render2D::init();

		controller.set_camera(0, 1, 0, 1);

		computeShader = Shader::load_comp("assets/shaders/game_of_life.comp");

		//compIn = Texture2D::rgba(size, size, TextureFilter::NEAREST);
		//compIn.fill_random_greyscale();
		//compIn = Texture2D::load("assets/images/uv_checker.png", TextureFilter::NEAREST).value();
		compIn = Texture2D::rgba(size, size, TextureFilter::NEAREST);
		fill_random(compIn);

		size = compIn.width();
		compOut = Texture2D::rgba(size, size, TextureFilter::NEAREST);
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

		ImGui::InputInt("size", &size);

		if (ImGui::Button("reload texture")) {
			compIn = Texture2D::rgba(size, size, Atlas::TextureFilter::NEAREST);
			compOut = Texture2D::rgba(size, size, Atlas::TextureFilter::NEAREST);
			fill_random(compIn);
			//compIn = Texture2D::load("assets/images/uv_checker.png", TextureFilter::NEAREST).value();
		}
		ImGui::End();
	}

	void on_event(Atlas::Event &e) override {
		controller.on_event(e);
	}

};

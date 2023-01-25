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
	float blurSize = 10;

	void on_attach() override {
		using namespace Atlas;
		Render2D::init();
		controller.set_camera(0, (float)size, 0, (float)size);
		//texture = Atlas::Texture2D::load("assets/images/uv_checker.png", Atlas::TextureFilter::NEAREST).value();

		computeShader = Shader::load_comp("assets/shaders/tex.comp");

		//compOut = Texture2D::rgba(1024, 1024, TextureFilter::NEAREST);
		compOut = Texture2D::load("assets/images/uv_checker.png", TextureFilter::NEAREST).value();

		computeShader.bind("imgOutput", compOut, TextureUsage::READ | TextureUsage::WRITE);
		computeShader.set("width", (float)compOut.width());
		computeShader.set("size", blurSize);
	}

	void on_detach() override {
	}

	void on_update(Atlas::Timestep ts) override {
		using namespace Atlas;
		ATL_EVENT("layer update");

		controller.on_update(ts);

		Render2D::set_camera(controller.get_camera());
		Render2D::reset_stats();

		computeShader.set("time", Application::get_time());
		Shader::dispatch(computeShader, compOut.width() / 32, compOut.height() / 32, 1);

		memory_barrier(Barrier::IMAGE_ACCESS);

		Render::begin(Application::get_viewport_color());

		{
			ATL_EVENT("draw triangles");
			for (int i = 0; i < size; i++) {
				for (int j = 0; j < size; j++) {
					Render2D::rect({ i, j }, { 1, 1 }, compOut);
				}
			}
		}

		Render2D::flush();

		Render2D::RenderStats stats = Render2D::get_stats();
		ImGui::Begin("RenderStats");
		ImGui::Text("draw calls: %d", stats.drawCalls);
		ImGui::Text("tris drawn: %d", stats.triangleCount);
		ImGui::End();

		Render::end();

	}

	void on_imgui() override {
		using namespace Atlas;

		ImGui::Begin("output");
		ImGui::Image(compOut, { 255, 255 });
		ImGui::End();

		ImGui::Begin("Settings");
		int tempSize = size;
		ImGui::DragInt("quad count", &tempSize, .1f, 1, 1000);
		if (tempSize != size) {
			size = tempSize;
			controller.set_camera(0, (float)size, 0, (float)size);
		}

		if (ImGui::Button("Reload texture")) {
			compOut = Texture2D::load("assets/images/uv_checker.png", TextureFilter::NEAREST).value();
			computeShader.bind("imgOutput", compOut, TextureUsage::READ | TextureUsage::WRITE);
		}

		ImGui::SliderFloat("size", &blurSize, 0, 10);
		computeShader.set("size", blurSize);

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

#include <iostream>
#include "application.h"

#include "RenderApi.h"
#include "Render2D.h"

using namespace Atlas;

struct Agent {
	glm::vec2 pos;
	glm::vec2 vel;
};

class Sandbox : public Atlas::Layer {

	Buffer agents;
	Texture2D img;
	Shader agentShader;
	Shader blurShader;

	OrthographicCameraController controller = OrthographicCameraController();

	void on_attach() override {
		controller.set_camera(0, 1, 0, 1);
		Render2D::init();

		img = Texture2D::rgba(128, 128, TextureFilter::NEAREST);
		img.fill(RGBA(0, 0, 0, 255));

		const uint32_t agentCount = 128;
		std::vector<Agent> agentsData(agentCount);
		for (auto &agent : agentsData) {
			//agent.pos = Random::get<glm::vec2>(-1, 1);
			agent.pos = glm::vec2(0.5, 0.5);
			agent.vel = glm::normalize(Random::get<glm::vec2>(-1, 1));
		}

		agents = Buffer::create(BufferType::STORAGE, agentsData.data(), agentsData.size() * sizeof(Agent));
		agentShader = Shader::load_comp("assets/shaders/agents.comp");
		blurShader = Shader::load_comp("assets/shaders/blur.comp");

		agentShader.bind("Agents", agents);
		agentShader.bind("outImg", img, TextureUsage::WRITE);

		blurShader.bind("img", img, TextureUsage::READ | TextureUsage::WRITE);
	}

	void on_update(Timestep ts) override {


		Shader::dispatch(blurShader, img.width() / 32, img.height() / 32, 1);
		memory_barrier(Barrier::IMAGE_ACCESS);


		//const int numWorkgroupsX = static_cast<int>(std::ceil(std::sqrt(agents.size()) / 32));
		//const int numWorkgroupsY = static_cast<int>(std::ceil(std::sqrt(agents.size()) / 32));

		Shader::dispatch(agentShader, 128, 1, 1);
		memory_barrier(Barrier::ALL);

		bool updated = true;

		controller.on_update(ts);
		Render2D::set_camera(controller.get_camera());
		Render::begin(Application::get_viewport_color());

		Render2D::square({ 0, 0 }, 1, img);

		Render2D::flush();
		Render::end();
	}

	void on_imgui() override {

	}

	void on_event(Event &e) override {
		controller.on_event(e);
	}
};

int main() {
	Atlas::Application app = Atlas::Application::default();

	app.push_layer(make_ref<Sandbox>());
	app.run();

}
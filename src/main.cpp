#include <iostream>
#include "application.h"

#include "RenderApi.h"
#include "Render2D.h"

#define PI 3.1415926535

using namespace Atlas;

struct Agent {
	glm::ivec2 speciesMask;
	glm::vec2 pos;
	float angle;
	int index;
};

struct SimSettings {
	glm::vec4 color;
	float moveSpeed;
	float turnSpeed;
	float sensorAngle;
	float sensorDistance;
	float sensorSize;
	float randomStrength;
};

struct BlurSettings {
	float evaporationSpeed;
	float difuseSpeed;
	int kernelSize;
};

struct GlobalSettings {
	SimSettings sim;
	BlurSettings blur;
};

class Sandbox : public Atlas::Layer {

	Buffer agents;
	Texture2D img;
	Shader agentShader;
	Shader blurShader;

	GlobalSettings settings;

	const uint32_t agentCount = 50000;

	OrthographicCameraController controller = OrthographicCameraController();

	void reset_settings() {
		settings.sim.moveSpeed = 0.5f; //1
		settings.sim.turnSpeed = 0.1f; //1.369
		settings.sim.sensorAngle = 0.2f; //0.39
		settings.sim.sensorDistance = 20.f; //20
		settings.sim.sensorSize = 2.f; //20
		settings.sim.randomStrength = 1.f;
		settings.sim.color = glm::vec4(1, 1, 1, 1);

		settings.blur.difuseSpeed = 0.1f; //0.1
		settings.blur.evaporationSpeed = 0.01f; //0.01
		settings.blur.kernelSize = 1;
	}

	void on_attach() override {
		controller.set_camera(0, 1, 0, 1);
		Render2D::init();

		const uint32_t imgSize = 128 * 5;
		img = Texture2D::rgba(imgSize, imgSize, TextureFilter::NEAREST);

		std::vector<Agent> agentsData(agentCount, Agent{});
		for (auto &agent : agentsData) {
			//agent.pos = Random::get<glm::vec2>(0, imgSize);
			agent.pos = glm::vec2(imgSize / 2, imgSize / 2);
			agent.angle = Random::get<float>(0.f, 2.f * PI);
			agent.index = 0;

			int species = Random::get<int>(0, 1);
			agent.speciesMask[species] = 1;
		}

		agents = Buffer::create(BufferType::STORAGE, agentsData.data(), agentsData.size() * sizeof(Agent));
		agentShader = Shader::load_comp("assets/shaders/agents.comp");
		blurShader = Shader::load_comp("assets/shaders/blur.comp");

		agentShader.bind("Agents", agents);
		agentShader.bind("outImg", img, TextureUsage::WRITE);

		blurShader.bind("img", img, TextureUsage::READ | TextureUsage::WRITE);

		reset_settings();

		{
			Buffer sim = Buffer::storage(settings.sim);
			agentShader.bind("settingsBuffer", sim);

			Buffer blur = Buffer::storage(settings.blur);
			blurShader.bind("settingsBuffer", blur);
		}
	}

	void on_update(Timestep ts) override {


		Shader::dispatch(blurShader, img.width() / 32, img.height() / 32, 1);
		memory_barrier(Barrier::IMAGE_ACCESS);

		Shader::dispatch(agentShader, (agentCount + 1023) / 1024, 1, 1);
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
		show_settings();
	}

	void show_settings() {

		bool updateSim = false;
		bool updateBlur = false;

		SimSettings simSettings = settings.sim;
		BlurSettings blurSettings = settings.blur;


		ImGui::Begin("Settings");

		ImGui::Text("Agents");
		ImGui::DragFloat("move speed", &simSettings.moveSpeed, 0.001, 0, 5);
		ImGui::DragFloat("turn speed", &simSettings.turnSpeed, 0.001, 0, 5);
		ImGui::DragFloat("sensor angle", &simSettings.sensorAngle, 0.001, 0, 5);
		ImGui::DragFloat("sensor distance", &simSettings.sensorDistance, 0.01, 0, 100);
		ImGui::DragFloat("sensor size", &simSettings.sensorSize, 0.002, 0, 10);
		ImGui::DragFloat("random strength", &simSettings.randomStrength, 0.001, 0, 5);
		ImGui::ColorEdit3("color", &simSettings.color[0]);

		ImGui::Text("Blur");
		ImGui::DragFloat("evaporation", &blurSettings.evaporationSpeed, 0.0001, 0, 1);
		ImGui::DragFloat("difuse", &blurSettings.difuseSpeed, 0.0001, 0, 1);
		ImGui::DragInt("kernel size", &blurSettings.kernelSize, 1, 0, 10);

		if (simSettings.moveSpeed != settings.sim.moveSpeed
			|| simSettings.turnSpeed != settings.sim.turnSpeed
			|| simSettings.sensorAngle != settings.sim.sensorAngle
			|| simSettings.sensorDistance != settings.sim.sensorDistance
			|| simSettings.sensorSize != settings.sim.sensorSize
			|| simSettings.randomStrength != settings.sim.randomStrength
			|| simSettings.color != settings.sim.color) {
			updateSim = true;
			settings.sim = simSettings;
		}

		if (blurSettings.evaporationSpeed != settings.blur.evaporationSpeed
			|| blurSettings.difuseSpeed != settings.blur.difuseSpeed
			|| blurSettings.kernelSize != settings.blur.kernelSize) {
			updateBlur = true;
			settings.blur = blurSettings;
		}


		if (ImGui::Button("Reset Settings")) {
			reset_settings();
			updateSim = true;
			updateBlur = true;
		}

		if (updateSim) {
			Buffer sim = Buffer::storage(settings.sim);
			agentShader.bind("settingsBuffer", sim);
		}

		if (updateBlur) {
			Buffer blur = Buffer::storage(settings.blur);
			blurShader.bind("settingsBuffer", blur);
		}

		ImGui::End();
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
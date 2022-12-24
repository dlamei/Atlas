#include <iostream>

#include "window.h"
#include "gl_utils.h"
#include "application.h"

using namespace Atlas;

int main() {

	Application app;
	app.run();


	//WindowCreateInfo wInfo{};
	//wInfo.width = 1200;
	//wInfo.height = 900;
	//Window w(wInfo);

	//gl_utils::init_opengl();
	//gl_utils::init_imgui(w.get_native_window());
	//gl_utils::init();

	//while (!w.should_close()) {
	//	gl_utils::update();
	//	w.on_update();
	//}
}

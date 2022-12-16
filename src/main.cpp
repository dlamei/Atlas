#include <iostream>

#include "window.h"
#include "gl_utils.h"

using namespace Atlas;

int main() {

	WindowCreateInfo wInfo{};
	wInfo.width = 1200;
	wInfo.height = 900;
	Window w(wInfo);

	gl_utils::init();

	while (!w.should_close()) {
		w.on_update();
		gl_utils::update();
	}
}

#include <iostream>

#include "window.h"

using namespace Atlas;

int main() {

	WindowCreateInfo wInfo{};
	wInfo.width = 1200;
	wInfo.height = 900;
	Window w(wInfo);

	while (!w.should_close()) {
		w.on_update();
	}
}

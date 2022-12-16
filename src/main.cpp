#include <iostream>

#include "window.h"

int main() {

	WindowCreateInfo wInfo{};
	wInfo.width = 100;
	wInfo.height = 100;
	Window w(wInfo);
}

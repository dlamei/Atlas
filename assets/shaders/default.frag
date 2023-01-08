#version 450

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;

out vec4 fragColor;

uniform sampler2D tex;

void main() {
	fragColor = texture(tex, inUV) * inColor;
}


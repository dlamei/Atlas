#version 450

layout (location = 0) in vec2 inUV;

out vec4 fragColor;

uniform sampler2D tex;

void main() {
	fragColor = texture(tex, inUV);
}


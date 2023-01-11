#version 450 core

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;
layout (location = 2) flat in int inTexID;

uniform sampler2D uTextureSlots[32];

out vec4 outFragColor;

void main() {
	outFragColor = texture(uTextureSlots[inTexID], inUV) * inColor;
}


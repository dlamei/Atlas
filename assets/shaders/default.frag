#version 450 core

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;
layout (location = 2) flat in int inTexID;
layout (location = 3) flat in int inIsEllipse;

uniform sampler2D uTextureSlots[32];

out vec4 outFragColor;

void main() {
	vec2 center = inUV * 2 - vec2(1, 1);
	float dist = center.x * center.x + center.y * center.y;
	float circle_alpha = smoothstep(0.0, 0.002, 1 - dist);
	outFragColor = texture(uTextureSlots[inTexID], inUV) * inColor;

	if (inIsEllipse == 1) {
		outFragColor.a *= circle_alpha;
	}
}

#version 450

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec2 vUV;

layout (location = 0) out vec2 outUV;

void main() {
	gl_Position = vec4(vPos.x, vPos.y, vPos.z, 1.0f);
	outUV = vPos.xy;
}


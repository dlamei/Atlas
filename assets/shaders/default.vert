#version 450

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec2 vUV;

layout (location = 0) out vec2 outUV;

layout (std140) uniform CameraBuffer {
	mat4 viewProj;
};

void main() {
	//gl_Position = camera.viewProj * vec4(vPos, 1.0f);
	gl_Position = viewProj * vec4(vPos, 1.0f);
	outUV = vUV;
}


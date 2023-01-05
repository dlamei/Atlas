#version 450

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec2 vUV;

layout (location = 0) out vec2 outUV;

struct Camera {
	mat4 viewProj;
};

layout (std140, binding = 3) uniform CameraBuffer {
	Camera cam;
};

void main() {
	//gl_Position = camera.viewProj * vec4(vPos, 1.0f);
	gl_Position = cam.viewProj * vec4(vPos, 1.0f);
	outUV = vUV;
}


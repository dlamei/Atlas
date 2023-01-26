#version 450 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec2 vUV;
layout (location = 2) in vec4 vColor;
layout (location = 3) in int vTexID;
layout (location = 4) in int vIsEllipse;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;
layout (location = 2) out int outTexID;
layout (location = 3) out int outIsEllipse;

struct Camera {
	mat4 viewProj;
};

layout (std140) uniform CameraBuffer {
	Camera cam;
};

void main() {
	gl_Position = cam.viewProj * vec4(vPos, 1.0f);
	outUV = vUV;
	outColor = vColor;
	outTexID = vTexID;
	outIsEllipse = vIsEllipse;
}


#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUv;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vPosition;
out vec3 vNormal;
out vec2 vUv;

void main() {
    vec4 position = uModel * vec4(aPosition, 1.0);
    vPosition = position.xyz;
    vNormal = transpose(inverse(mat3(uModel))) * aNormal;
    vUv = aUv;
    gl_Position = uProjection * uView * position;
}

#version 330 core
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUv;
uniform mat4 uModel,uView,uProjection,uLightMatrix;
out vec3 vPosition;
out vec3 vNormal;
out vec2 vUv;
out vec4 vLightPosition;
void main(){
    vec4 p=uModel*vec4(aPosition,1);
    vPosition=p.xyz;vNormal=transpose(inverse(mat3(uModel)))*aNormal;vUv=aUv;
    vLightPosition=uLightMatrix*p;gl_Position=uProjection*uView*p;
}

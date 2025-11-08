#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;

uniform mat4 uModel, uView, uProj;

out vec3 vPos;
out vec3 vNormal;

void main(){
    vec4 wp = uModel * vec4(aPos,1.0);
    vPos = wp.xyz;
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    gl_Position = uProj * uView * wp;
}

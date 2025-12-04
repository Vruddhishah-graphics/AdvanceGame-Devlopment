#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoord;
    mat3 TBN;
} vs_out;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vs_out.FragPos = worldPos.xyz;
    vs_out.TexCoord = aTexCoord;
    
    // Calculate TBN matrix for normal mapping
    vec3 T = normalize(vec3(uModel * vec4(aTangent, 0.0)));
    vec3 B = normalize(vec3(uModel * vec4(aBitangent, 0.0)));
    vec3 N = normalize(vec3(uModel * vec4(aNormal, 0.0)));
    vs_out.TBN = mat3(T, B, N);
    
    gl_Position = uProj * uView * worldPos;
}
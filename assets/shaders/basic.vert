#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;

uniform mat4 uModel, uView, uProj;

out vec3 vPos;
out vec3 vNormal;
out vec2 vTexCoord;

void main(){
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vPos = worldPos.xyz;
    
    // Proper normal transformation (handles non-uniform scaling)
    mat3 normalMatrix = mat3(transpose(inverse(uModel)));
    vNormal = normalize(normalMatrix * aNormal);
    
    // Generate better texture coordinates based on position
    // For walls and floor, use world-space XZ coordinates
    vTexCoord = vec2(vPos.x * 0.5 + 0.5, vPos.z * 0.5 + 0.5);
    
    gl_Position = uProj * uView * worldPos;
}
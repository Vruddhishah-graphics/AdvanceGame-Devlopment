#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in ivec4 aBoneIDs;
layout(location = 5) in vec4 aBoneWeights;

const int MAX_BONES = 100;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoord;
    mat3 TBN;
} vs_out;

uniform mat4 uBoneTransforms[MAX_BONES];
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

void main() {
    // GPU Skinning
    mat4 boneTransform = mat4(0.0);
    for(int i = 0; i < 4; i++) {
        if(aBoneIDs[i] >= 0) {
            boneTransform += uBoneTransforms[aBoneIDs[i]] * aBoneWeights[i];
        }
    }
    
    vec4 skinnedPos = boneTransform * vec4(aPos, 1.0);
    vec4 worldPos = uModel * skinnedPos;
    vs_out.FragPos = worldPos.xyz;
    vs_out.TexCoord = aTexCoord;
    
    // Transform normal with bone transform
    mat3 normalMatrix = transpose(inverse(mat3(uModel * boneTransform)));
    vec3 N = normalize(normalMatrix * aNormal);
    vec3 T = normalize(normalMatrix * aTangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    vs_out.TBN = mat3(T, B, N);
    
    gl_Position = uProj * uView * worldPos;
}
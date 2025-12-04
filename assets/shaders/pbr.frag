#version 330 core

const float PI = 3.14159265359;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoord;
    mat3 TBN;
} fs_in;

out vec4 FragColor;

// Material properties
uniform sampler2D uAlbedoMap;
uniform sampler2D uNormalMap;
uniform sampler2D uMetallicMap;
uniform sampler2D uRoughnessMap;
uniform sampler2D uAOMap;

uniform vec3 uAlbedo;
uniform float uMetallic;
uniform float uRoughness;
uniform bool uUseAlbedoMap;
uniform bool uUseNormalMap;
uniform bool uUseMetallicMap;
uniform bool uUseRoughnessMap;
uniform bool uUseAOMap;

// Lighting
uniform vec3 uCamPos;
uniform vec3 uLightPositions[4];
uniform vec3 uLightColors[4];
uniform int uNumLights;

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return nom / denom;
}

// Geometry Function (Smith's Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

// Fresnel-Schlick approximation
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    // Sample material properties
    vec3 albedo = uUseAlbedoMap ? 
        pow(texture(uAlbedoMap, fs_in.TexCoord).rgb, vec3(2.2)) : uAlbedo;
    
    float metallic = uUseMetallicMap ? 
        texture(uMetallicMap, fs_in.TexCoord).r : uMetallic;
    
    float roughness = uUseRoughnessMap ? 
        texture(uRoughnessMap, fs_in.TexCoord).r : uRoughness;
    
    float ao = uUseAOMap ? 
        texture(uAOMap, fs_in.TexCoord).r : 1.0;
    
    // Normal mapping
    vec3 N;
    if (uUseNormalMap) {
        N = texture(uNormalMap, fs_in.TexCoord).rgb;
        N = N * 2.0 - 1.0;
        N = normalize(fs_in.TBN * N);
    } else {
        N = normalize(fs_in.TBN[2]);
    }
    
    vec3 V = normalize(uCamPos - fs_in.FragPos);
    
    // Calculate reflectance at normal incidence
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    // Reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < uNumLights; ++i) {
        vec3 L = normalize(uLightPositions[i] - fs_in.FragPos);
        vec3 H = normalize(V + L);
        float distance = length(uLightPositions[i] - fs_in.FragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = uLightColors[i] * attenuation;
        
        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
        
        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;
        
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
    
    // Ambient lighting
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;
    
    // HDR tonemapping
    color = color / (color + vec3(1.0));
    
    // Gamma correction
    color = pow(color, vec3(1.0/2.2));
    
    FragColor = vec4(color, 1.0);
}
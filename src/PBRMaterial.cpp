// PBRMaterial.cpp - Complete PBR Material System
#include "game/PBRMaterial.h"
#include <iostream>

// Use stb_image for texture loading
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace game {

    // ============================================================================
    // PBRTexture Implementation
    // ============================================================================

    PBRTexture::~PBRTexture() {
        if (id) glDeleteTextures(1, &id);
    }

    bool PBRTexture::LoadFromFile(const std::string& path) {
        int nrChannels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

        if (!data) {
            std::cerr << "Failed to load texture: " << path << std::endl;
            return false;
        }

        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        GLenum format = GL_RGB;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;

        // Determine if texture is sRGB (albedo textures should be sRGB)
        bool isSRGB = (path.find("albedo") != std::string::npos ||
            path.find("diffuse") != std::string::npos ||
            path.find("color") != std::string::npos);

        GLenum internalFormat = isSRGB ? GL_SRGB8_ALPHA8 : format;
        if (format == GL_RGB && isSRGB) internalFormat = GL_SRGB8;

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height,
            0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // High quality filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Anisotropic filtering if available
        float maxAnisotropy;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);

        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);

        std::cout << "  Loaded PBR texture: " << path << " (" << width << "x" << height
            << ", " << nrChannels << " channels)\n";
        return true;
    }

    void PBRTexture::Bind(int unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, id);
    }

    // ============================================================================
    // PBRMaterial Presets
    // ============================================================================

    PBRMaterial PBRMaterialLibrary::CreateWood() {
        PBRMaterial mat;

        // Warm brown wood color
        mat.albedoColor = glm::vec3(0.72f, 0.52f, 0.36f);
        mat.metallicValue = 0.0f;  // Wood is non-metallic
        mat.roughnessValue = 0.7f; // Slightly rough

        // Try to load textures (fallback to values if not found)
        mat.albedoMap = std::make_shared<PBRTexture>();
        if (!mat.albedoMap->LoadFromFile("assets/textures/wood_albedo.png")) {
            mat.albedoMap = nullptr;
        }

        mat.normalMap = std::make_shared<PBRTexture>();
        if (!mat.normalMap->LoadFromFile("assets/textures/wood_normal.png")) {
            mat.normalMap = nullptr;
        }

        mat.roughnessMap = std::make_shared<PBRTexture>();
        if (!mat.roughnessMap->LoadFromFile("assets/textures/wood_roughness.png")) {
            mat.roughnessMap = nullptr;
        }

        return mat;
    }

    PBRMaterial PBRMaterialLibrary::CreateFabric() {
        PBRMaterial mat;

        mat.albedoColor = glm::vec3(0.45f, 0.64f, 0.86f); // Blue fabric
        mat.metallicValue = 0.0f;
        mat.roughnessValue = 0.9f; // Very rough

        mat.albedoMap = std::make_shared<PBRTexture>();
        if (!mat.albedoMap->LoadFromFile("assets/textures/fabric_albedo.png")) {
            mat.albedoMap = nullptr;
        }

        mat.normalMap = std::make_shared<PBRTexture>();
        if (!mat.normalMap->LoadFromFile("assets/textures/fabric_normal.png")) {
            mat.normalMap = nullptr;
        }

        return mat;
    }

    PBRMaterial PBRMaterialLibrary::CreateMetal() {
        PBRMaterial mat;

        mat.albedoColor = glm::vec3(0.85f, 0.85f, 0.85f); // Silver
        mat.metallicValue = 1.0f;  // Fully metallic
        mat.roughnessValue = 0.2f; // Shiny

        mat.albedoMap = std::make_shared<PBRTexture>();
        if (!mat.albedoMap->LoadFromFile("assets/textures/metal_albedo.png")) {
            mat.albedoMap = nullptr;
        }

        mat.roughnessMap = std::make_shared<PBRTexture>();
        if (!mat.roughnessMap->LoadFromFile("assets/textures/metal_roughness.png")) {
            mat.roughnessMap = nullptr;
        }

        return mat;
    }

    PBRMaterial PBRMaterialLibrary::CreatePlastic() {
        PBRMaterial mat;

        mat.albedoColor = glm::vec3(0.9f, 0.2f, 0.2f); // Red plastic
        mat.metallicValue = 0.0f;
        mat.roughnessValue = 0.3f; // Somewhat shiny

        return mat;
    }

    PBRMaterial PBRMaterialLibrary::CreateCarpet() {
        PBRMaterial mat;

        mat.albedoColor = glm::vec3(0.6f, 0.3f, 0.2f); // Brown carpet
        mat.metallicValue = 0.0f;
        mat.roughnessValue = 0.95f; // Very rough

        return mat;
    }

    PBRMaterial PBRMaterialLibrary::CreateJerryFur() {
        PBRMaterial mat;

        mat.albedoColor = glm::vec3(0.92f, 0.92f, 1.0f); // Light gray/white
        mat.metallicValue = 0.0f;
        mat.roughnessValue = 0.8f; // Fur is quite rough

        return mat;
    }

    PBRMaterial PBRMaterialLibrary::CreateTomFur() {
        PBRMaterial mat;

        mat.albedoColor = glm::vec3(1.0f, 0.63f, 0.35f); // Orange-brown
        mat.metallicValue = 0.0f;
        mat.roughnessValue = 0.75f;

        return mat;
    }

    PBRMaterial PBRMaterialLibrary::CreateCheese() {
        PBRMaterial mat;

        mat.albedoColor = glm::vec3(1.0f, 0.95f, 0.2f); // Yellow cheese
        mat.metallicValue = 0.0f;
        mat.roughnessValue = 0.6f;
        mat.emissiveColor = glm::vec3(1.0f, 0.95f, 0.2f);
        mat.emissiveStrength = 0.3f; // Slight glow

        return mat;
    }

    PBRMaterial PBRMaterialLibrary::CreateGrass() {
        PBRMaterial mat;

        mat.albedoColor = glm::vec3(0.5f, 0.8f, 0.4f);
        mat.metallicValue = 0.0f;
        mat.roughnessValue = 0.85f;

        mat.albedoMap = std::make_shared<PBRTexture>();
        if (!mat.albedoMap->LoadFromFile("assets/textures/grass_albedo.png")) {
            mat.albedoMap = nullptr;
        }

        return mat;
    }

    PBRMaterial PBRMaterialLibrary::CreateStone() {
        PBRMaterial mat;

        mat.albedoColor = glm::vec3(0.7f, 0.7f, 0.7f);
        mat.metallicValue = 0.0f;
        mat.roughnessValue = 0.9f;

        mat.albedoMap = std::make_shared<PBRTexture>();
        if (!mat.albedoMap->LoadFromFile("assets/textures/stone_albedo.png")) {
            mat.albedoMap = nullptr;
        }

        mat.normalMap = std::make_shared<PBRTexture>();
        if (!mat.normalMap->LoadFromFile("assets/textures/stone_normal.png")) {
            mat.normalMap = nullptr;
        }

        return mat;
    }

    // ============================================================================
    // PBRRenderer - Renders objects with PBR materials
    // ============================================================================

    void PBRRenderer::Init() {
        // Load PBR shader
        pbrShader_ = std::make_unique<Shader>();

        // Try to load from files, fallback to embedded
        if (!pbrShader_->LoadFromFiles(
            "assets/shaders/pbr.vert",
            "assets/shaders/pbr.frag")) {

            std::cout << "Using embedded PBR shaders\n";
            // Use the embedded shaders from the guide
            pbrShader_->LoadFromStrings(GetEmbeddedPBRVertexShader().c_str(),
                GetEmbeddedPBRFragmentShader().c_str());
        }

        std::cout << "PBR Renderer initialized\n";
    }

    void PBRRenderer::SetLights(const std::vector<Light>& lights) {
        lights_ = lights;
    }

    void PBRRenderer::RenderMesh(const Mesh& mesh,
        const PBRMaterial& material,
        const glm::mat4& modelMatrix,
        const glm::mat4& viewMatrix,
        const glm::mat4& projMatrix,
        const glm::vec3& camPos) {

        pbrShader_->Use();

        // Set matrices
        pbrShader_->SetMat4("uModel", modelMatrix);
        pbrShader_->SetMat4("uView", viewMatrix);
        pbrShader_->SetMat4("uProj", projMatrix);
        pbrShader_->SetVec3("uCamPos", camPos);

        // Set material properties
        pbrShader_->SetVec3("uAlbedo", material.albedoColor);
        pbrShader_->SetFloat("uMetallic", material.metallicValue);
        pbrShader_->SetFloat("uRoughness", material.roughnessValue);

        // Bind textures
        int texUnit = 0;

        if (material.albedoMap && material.albedoMap->id) {
            material.albedoMap->Bind(texUnit);
            pbrShader_->SetInt("uAlbedoMap", texUnit);
            pbrShader_->SetBool("uUseAlbedoMap", true);
            texUnit++;
        }
        else {
            pbrShader_->SetBool("uUseAlbedoMap", false);
        }

        if (material.normalMap && material.normalMap->id) {
            material.normalMap->Bind(texUnit);
            pbrShader_->SetInt("uNormalMap", texUnit);
            pbrShader_->SetBool("uUseNormalMap", true);
            texUnit++;
        }
        else {
            pbrShader_->SetBool("uUseNormalMap", false);
        }

        if (material.metallicMap && material.metallicMap->id) {
            material.metallicMap->Bind(texUnit);
            pbrShader_->SetInt("uMetallicMap", texUnit);
            pbrShader_->SetBool("uUseMetallicMap", true);
            texUnit++;
        }
        else {
            pbrShader_->SetBool("uUseMetallicMap", false);
        }

        if (material.roughnessMap && material.roughnessMap->id) {
            material.roughnessMap->Bind(texUnit);
            pbrShader_->SetInt("uRoughnessMap", texUnit);
            pbrShader_->SetBool("uUseRoughnessMap", true);
            texUnit++;
        }
        else {
            pbrShader_->SetBool("uUseRoughnessMap", false);
        }

        if (material.aoMap && material.aoMap->id) {
            material.aoMap->Bind(texUnit);
            pbrShader_->SetInt("uAOMap", texUnit);
            pbrShader_->SetBool("uUseAOMap", true);
            texUnit++;
        }
        else {
            pbrShader_->SetBool("uUseAOMap", false);
        }

        // Set lights
        pbrShader_->SetInt("uNumLights", std::min((int)lights_.size(), 4));
        for (size_t i = 0; i < std::min(lights_.size(), size_t(4)); ++i) {
            std::string base = "uLightPositions[" + std::to_string(i) + "]";
            pbrShader_->SetVec3(base, lights_[i].position);

            base = "uLightColors[" + std::to_string(i) + "]";
            pbrShader_->SetVec3(base, lights_[i].color);
        }

        // Render mesh
        drawMesh(mesh);
    }

    std::string PBRRenderer::GetEmbeddedPBRVertexShader() {
        return R"(
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoord;
    vec3 Normal;
} vs_out;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vs_out.FragPos = worldPos.xyz;
    vs_out.TexCoord = aTexCoord;
    vs_out.Normal = mat3(transpose(inverse(uModel))) * aNormal;
    
    gl_Position = uProj * uView * worldPos;
}
)";
    }

    std::string PBRRenderer::GetEmbeddedPBRFragmentShader() {
        // Return the complete PBR fragment shader from the guide
        return R"(
#version 330 core

const float PI = 3.14159265359;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoord;
    vec3 Normal;
} fs_in;

out vec4 FragColor;

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

uniform vec3 uCamPos;
uniform vec3 uLightPositions[4];
uniform vec3 uLightColors[4];
uniform int uNumLights;

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

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 albedo = uUseAlbedoMap ? 
        pow(texture(uAlbedoMap, fs_in.TexCoord).rgb, vec3(2.2)) : uAlbedo;
    
    float metallic = uUseMetallicMap ? 
        texture(uMetallicMap, fs_in.TexCoord).r : uMetallic;
    
    float roughness = uUseRoughnessMap ? 
        texture(uRoughnessMap, fs_in.TexCoord).r : uRoughness;
    
    float ao = uUseAOMap ? 
        texture(uAOMap, fs_in.TexCoord).r : 1.0;
    
    vec3 N = normalize(fs_in.Normal);
    vec3 V = normalize(uCamPos - fs_in.FragPos);
    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < uNumLights; ++i) {
        vec3 L = normalize(uLightPositions[i] - fs_in.FragPos);
        vec3 H = normalize(V + L);
        float distance = length(uLightPositions[i] - fs_in.FragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = uLightColors[i] * attenuation;
        
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
    
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;
    
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));
    
    FragColor = vec4(color, 1.0);
}
)";
    }

} // namespace game
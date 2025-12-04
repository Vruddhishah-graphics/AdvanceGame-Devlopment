#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <memory>

namespace game {

    class PBRTexture {
    public:
        GLuint id = 0;
        int width = 0;
        int height = 0;

        bool LoadFromFile(const std::string& path);
        void Bind(int unit) const;
        ~PBRTexture();
    };

    struct PBRMaterial {
        // Albedo (base color)
        std::shared_ptr<PBRTexture> albedoMap;
        glm::vec3 albedoColor = glm::vec3(1.0f);

        // Normal mapping
        std::shared_ptr<PBRTexture> normalMap;

        // Metallic (0 = dielectric, 1 = metal)
        std::shared_ptr<PBRTexture> metallicMap;
        float metallicValue = 0.0f;

        // Roughness (0 = smooth, 1 = rough)
        std::shared_ptr<PBRTexture> roughnessMap;
        float roughnessValue = 0.5f;

        // Ambient occlusion
        std::shared_ptr<PBRTexture> aoMap;

        // Emissive
        std::shared_ptr<PBRTexture> emissiveMap;
        glm::vec3 emissiveColor = glm::vec3(0.0f);
        float emissiveStrength = 0.0f;
    };

    class PBRMaterialLibrary {
    public:
        static PBRMaterial CreateWood();
        static PBRMaterial CreateFabric();
        static PBRMaterial CreateMetal();
        static PBRMaterial CreatePlastic();
        static PBRMaterial CreateCarpet();
    };

} // namespace game
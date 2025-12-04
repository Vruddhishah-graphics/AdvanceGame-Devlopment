#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

namespace game {

    class ShadowMap {
    public:
        ShadowMap(int width = 2048, int height = 2048);
        ~ShadowMap();

        void BeginShadowPass(const glm::mat4& lightSpaceMatrix);
        void EndShadowPass();
        void BindForReading(int textureUnit);

        glm::mat4 GetLightSpaceMatrix(const glm::vec3& lightPos,
            const glm::vec3& targetPos);

    private:
        GLuint fbo_;
        GLuint depthMap_;
        int width_, height_;
    };

} // namespace game
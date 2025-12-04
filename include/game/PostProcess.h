#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

namespace game {

    class GBuffer {
    public:
        void Init(int width, int height);
        void BindForWriting();
        void BindForReading();

        GLuint positionTexture;
        GLuint normalTexture;
        GLuint albedoTexture;
        GLuint metallicRoughnessTexture;
        GLuint fbo;
    };

    class PostProcessor {
    public:
        void Init(int width, int height);

        // Effects
        void ApplyBloom(GLuint sourceTexture);
        void ApplyToneMapping(GLuint hdrTexture, float exposure);
        void ApplyFXAA(GLuint sourceTexture);

        GLuint GetFinalTexture() const { return finalTexture_; }

    private:
        void CreateFramebuffer();
        void CreateBloomBuffers();

        GLuint finalTexture_;
        GLuint bloomTexture_;
        // ... more buffers
    };

} // namespace game
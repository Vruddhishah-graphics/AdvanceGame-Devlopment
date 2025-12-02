#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

namespace game {

    class UIRenderer {
    public:
        UIRenderer();
        ~UIRenderer();

        void Init(int screenWidth, int screenHeight);
        void SetScreenSize(int width, int height);

        // Render UI elements
        void RenderHealthBar(float x, float y, float width, float height,
            float percentage, const glm::vec3& color);

        void RenderRect(float x, float y, float width, float height,
            const glm::vec4& color);

        void RenderBorder(float x, float y, float width, float height,
            float thickness, const glm::vec4& color);

        // Text rendering (simplified - uses rectangles for now)
        void RenderText(const std::string& text, float x, float y, float scale,
            const glm::vec3& color);

        void BeginUI();
        void EndUI();

    private:
        void InitShaders();
        void InitMesh();

        GLuint shader_;
        GLuint vao_;
        GLuint vbo_;

        GLint uProjection_;
        GLint uModel_;
        GLint uColor_;

        int screenWidth_;
        int screenHeight_;

        glm::mat4 projection_;
    };

} // namespace game
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

        void BeginUI();
        void EndUI();

        // Basic shapes
        void RenderRect(float x, float y, float width, float height, const glm::vec4& color);
        void RenderBorder(float x, float y, float width, float height, float thickness, const glm::vec4& color);
        void RenderHealthBar(float x, float y, float width, float height, float percentage, const glm::vec3& color);

        // Simple bitmap text rendering
        void RenderText(const std::string& text, float x, float y, float scale, const glm::vec3& color);
        void RenderCenteredText(const std::string& text, float y, float scale, const glm::vec3& color);

    private:
        void InitShaders();
        void InitMesh();
        void InitFont();

        // Render a single character
        void RenderChar(char c, float x, float y, float scale, const glm::vec4& color);

        GLuint shader_;
        GLuint vao_, vbo_;
        GLint uProjection_, uModel_, uColor_;

        glm::mat4 projection_;
        int screenWidth_, screenHeight_;

        // Simple bitmap font (8x8 pixels per character)
        static const int CHAR_WIDTH = 8;
        static const int CHAR_HEIGHT = 8;
    };

} // namespace game
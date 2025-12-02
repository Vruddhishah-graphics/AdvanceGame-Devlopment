#include "game/UIRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace game {

    UIRenderer::UIRenderer()
        : shader_(0), vao_(0), vbo_(0),
        uProjection_(-1), uModel_(-1), uColor_(-1),
        screenWidth_(1280), screenHeight_(720) {
    }

    UIRenderer::~UIRenderer() {
        if (vao_) glDeleteVertexArrays(1, &vao_);
        if (vbo_) glDeleteBuffers(1, &vbo_);
        if (shader_) glDeleteProgram(shader_);
    }

    void UIRenderer::Init(int screenWidth, int screenHeight) {
        screenWidth_ = screenWidth;
        screenHeight_ = screenHeight;

        InitShaders();
        InitMesh();
        SetScreenSize(screenWidth, screenHeight);

        std::cout << "  ✓ UI Renderer initialized\n";
    }

    void UIRenderer::InitShaders() {
        const char* vertSrc = R"(
            #version 330 core
            layout(location = 0) in vec2 aPos;
            
            uniform mat4 uProjection;
            uniform mat4 uModel;
            
            void main() {
                gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
            }
        )";

        const char* fragSrc = R"(
            #version 330 core
            uniform vec4 uColor;
            out vec4 FragColor;
            
            void main() {
                FragColor = uColor;
            }
        )";

        // Compile vertex shader
        GLuint vert = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vert, 1, &vertSrc, nullptr);
        glCompileShader(vert);

        GLint success;
        glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(vert, 512, nullptr, log);
            std::cerr << "UI vertex shader error: " << log << std::endl;
        }

        // Compile fragment shader
        GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag, 1, &fragSrc, nullptr);
        glCompileShader(frag);

        glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(frag, 512, nullptr, log);
            std::cerr << "UI fragment shader error: " << log << std::endl;
        }

        // Link program
        shader_ = glCreateProgram();
        glAttachShader(shader_, vert);
        glAttachShader(shader_, frag);
        glLinkProgram(shader_);

        glGetProgramiv(shader_, GL_LINK_STATUS, &success);
        if (!success) {
            char log[512];
            glGetProgramInfoLog(shader_, 512, nullptr, log);
            std::cerr << "UI shader link error: " << log << std::endl;
        }

        glDeleteShader(vert);
        glDeleteShader(frag);

        // Get uniform locations
        uProjection_ = glGetUniformLocation(shader_, "uProjection");
        uModel_ = glGetUniformLocation(shader_, "uModel");
        uColor_ = glGetUniformLocation(shader_, "uColor");
    }

    void UIRenderer::InitMesh() {
        // Simple quad
        float vertices[] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
        };

        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        glBindVertexArray(0);
    }

    void UIRenderer::SetScreenSize(int width, int height) {
        screenWidth_ = width;
        screenHeight_ = height;
        projection_ = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
    }

    void UIRenderer::BeginUI() {
        glUseProgram(shader_);
        glUniformMatrix4fv(uProjection_, 1, GL_FALSE, glm::value_ptr(projection_));

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void UIRenderer::EndUI() {
        glEnable(GL_DEPTH_TEST);
        glUseProgram(0);
    }

    void UIRenderer::RenderRect(float x, float y, float width, float height,
        const glm::vec4& color) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, y, 0.0f));
        model = glm::scale(model, glm::vec3(width, height, 1.0f));

        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(model));
        glUniform4fv(uColor_, 1, glm::value_ptr(color));

        glBindVertexArray(vao_);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);
    }

    void UIRenderer::RenderBorder(float x, float y, float width, float height,
        float thickness, const glm::vec4& color) {
        // Top
        RenderRect(x, y, width, thickness, color);
        // Bottom
        RenderRect(x, y + height - thickness, width, thickness, color);
        // Left
        RenderRect(x, y, thickness, height, color);
        // Right
        RenderRect(x + width - thickness, y, thickness, height, color);
    }

    void UIRenderer::RenderHealthBar(float x, float y, float width, float height,
        float percentage, const glm::vec3& color) {
        percentage = glm::clamp(percentage, 0.0f, 1.0f);

        // Background (dark)
        RenderRect(x, y, width, height, glm::vec4(0.2f, 0.2f, 0.2f, 0.8f));

        // Health fill
        if (percentage > 0.0f) {
            glm::vec4 fillColor(color.r, color.g, color.b, 0.9f);
            RenderRect(x + 2, y + 2, (width - 4) * percentage, height - 4, fillColor);
        }

        // Border
        RenderBorder(x, y, width, height, 2.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    void UIRenderer::RenderText(const std::string& text, float x, float y,
        float scale, const glm::vec3& color) {
        // Simplified text rendering using small rectangles for each character
        // In a full game, you'd use a proper font texture atlas

        float charWidth = 8.0f * scale;
        float charHeight = 12.0f * scale;
        float spacing = 2.0f * scale;

        for (size_t i = 0; i < text.length(); ++i) {
            float cx = x + i * (charWidth + spacing);

            // Render a simple box for each character (placeholder)
            RenderRect(cx, y, charWidth, charHeight,
                glm::vec4(color.r, color.g, color.b, 0.8f));
        }
    }

} // namespace game
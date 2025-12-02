#pragma once
#include <GL/glew.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <string>
#include <iostream>

namespace game {

    class Texture {
    public:
        Texture() : id_(0), width_(0), height_(0) {}

        ~Texture() {
            if (id_) glDeleteTextures(1, &id_);
        }

        // Load texture from file (stub - would need SOIL or stb_image)
        bool LoadFromFile(const std::string& path) {
            // Placeholder - returns false to trigger procedural generation
            return false;
        }

        void GenerateGrass() {
            const int size = 256;
            width_ = height_ = size;
            std::vector<unsigned char> data(size * size * 3);

            for (int y = 0; y < size; ++y) {
                for (int x = 0; x < size; ++x) {
                    int idx = (y * size + x) * 3;

                    // Use Perlin-like noise for natural grass texture
                    float noise = (std::rand() % 50) / 255.0f;
                    float stripe = std::sin(y * 0.3f) * 0.1f;

                    data[idx + 0] = static_cast<unsigned char>(30 + noise * 255 + stripe * 50);   // R - darker green
                    data[idx + 1] = static_cast<unsigned char>(120 + noise * 255);  // G - main green
                    data[idx + 2] = static_cast<unsigned char>(40 + noise * 255);   // B
                }
            }

            CreateTexture(data.data());
        }

        void GenerateStone() {
            const int size = 256;
            width_ = height_ = size;
            std::vector<unsigned char> data(size * size * 3);

            for (int y = 0; y < size; ++y) {
                for (int x = 0; x < size; ++x) {
                    int idx = (y * size + x) * 3;

                    // Add some texture variation
                    float noise = (std::rand() % 80) / 255.0f;
                    float pattern = std::sin(x * 0.1f) * std::cos(y * 0.1f) * 0.15f;

                    unsigned char gray = static_cast<unsigned char>(100 + noise * 255 + pattern * 50);
                    data[idx + 0] = gray;
                    data[idx + 1] = gray;
                    data[idx + 2] = gray;
                }
            }

            CreateTexture(data.data());
        }

        void GenerateMetal() {
            const int size = 256;
            width_ = height_ = size;
            std::vector<unsigned char> data(size * size * 3);

            for (int y = 0; y < size; ++y) {
                for (int x = 0; x < size; ++x) {
                    int idx = (y * size + x) * 3;

                    // Metallic brushed effect
                    float noise = (std::rand() % 30) / 255.0f;
                    float streak = std::sin(x * 0.5f) * 0.1f;

                    unsigned char gray = static_cast<unsigned char>(160 + noise * 255 + streak * 30);
                    data[idx + 0] = gray;
                    data[idx + 1] = gray;
                    data[idx + 2] = gray;
                }
            }

            CreateTexture(data.data());
        }

        void GenerateCheckerboard() {
            const int size = 256;
            const int checks = 8;
            width_ = height_ = size;
            std::vector<unsigned char> data(size * size * 3);

            for (int y = 0; y < size; ++y) {
                for (int x = 0; x < size; ++x) {
                    int idx = (y * size + x) * 3;
                    bool white = ((x / (size / checks)) + (y / (size / checks))) % 2 == 0;

                    // Wood-like colors instead of pure B&W
                    if (white) {
                        data[idx + 0] = 180 + (std::rand() % 40);  // Light wood
                        data[idx + 1] = 140 + (std::rand() % 40);
                        data[idx + 2] = 100 + (std::rand() % 40);
                    }
                    else {
                        data[idx + 0] = 120 + (std::rand() % 30);  // Dark wood
                        data[idx + 1] = 80 + (std::rand() % 30);
                        data[idx + 2] = 50 + (std::rand() % 30);
                    }
                }
            }

            CreateTexture(data.data());
        }

        void Bind(int unit = 0) const {
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, id_);
        }

        GLuint GetID() const { return id_; }

    private:
        void CreateTexture(const unsigned char* data) {
            glGenTextures(1, &id_);
            glBindTexture(GL_TEXTURE_2D, id_);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0,
                GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glBindTexture(GL_TEXTURE_2D, 0);
        }

        GLuint id_;
        int width_;
        int height_;
    };

} // namespace game
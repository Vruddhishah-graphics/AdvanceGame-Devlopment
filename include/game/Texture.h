#pragma once
#include <GL/glew.h>
#include <vector>
#include <cmath>
#include <cstdlib>

namespace game {

    class Texture {
    public:
        Texture() : id_(0), width_(0), height_(0) {}

        ~Texture() {
            if (id_) glDeleteTextures(1, &id_);
        }

        void GenerateGrass() {
            const int size = 256;
            width_ = height_ = size;
            std::vector<unsigned char> data(size * size * 3);

            for (int y = 0; y < size; ++y) {
                for (int x = 0; x < size; ++x) {
                    int idx = (y * size + x) * 3;
                    float noise = (std::rand() % 50) / 255.0f;
                    data[idx + 0] = static_cast<unsigned char>(40 + noise * 255);   // R
                    data[idx + 1] = static_cast<unsigned char>(140 + noise * 255);  // G
                    data[idx + 2] = static_cast<unsigned char>(50 + noise * 255);   // B
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
                    float noise = (std::rand() % 80) / 255.0f;
                    unsigned char gray = static_cast<unsigned char>(100 + noise * 255);
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
                    float noise = (std::rand() % 30) / 255.0f;
                    unsigned char gray = static_cast<unsigned char>(160 + noise * 255);
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
                    unsigned char val = white ? 220 : 100;
                    data[idx + 0] = val;
                    data[idx + 1] = val - 20;
                    data[idx + 2] = val - 40;
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
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0,
                GL_RGB, GL_UNSIGNED_BYTE, data);

            glBindTexture(GL_TEXTURE_2D, 0);
        }

        GLuint id_;
        int width_;
        int height_;
    };

} // namespace game
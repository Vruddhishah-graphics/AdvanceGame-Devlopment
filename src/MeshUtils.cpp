#include "game/MeshUtils.h"
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct V { float x, y, z, nx, ny, nz; };

namespace game {

    static void setupAttribs() {
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void*)(3 * sizeof(float)));
    }

    Mesh makeBox() {
        std::vector<V> v; std::vector<unsigned int> i;
        auto face = [&](float ax, float ay, float az, float bx, float by, float bz, float cx, float cy, float cz, float dx, float dy, float dz,
            float nx, float ny, float nz) {
                unsigned int base = (unsigned int)v.size();
                v.push_back({ ax,ay,az, nx,ny,nz });
                v.push_back({ bx,by,bz, nx,ny,nz });
                v.push_back({ cx,cy,cz, nx,ny,nz });
                v.push_back({ dx,dy,dz, nx,ny,nz });
                i.insert(i.end(), { base + 0u,base + 1u,base + 2u, base + 2u,base + 3u,base + 0u });
            };
        float s = 0.5f;
        face(s, -s, -s, s, s, -s, s, s, s, s, -s, s, 1, 0, 0);
        face(-s, -s, s, -s, s, s, -s, s, -s, -s, -s, -s, -1, 0, 0);
        face(-s, s, -s, s, s, -s, s, s, s, -s, s, s, 0, 1, 0);
        face(-s, -s, s, s, -s, s, s, -s, -s, -s, -s, -s, 0, -1, 0);
        face(-s, -s, s, -s, s, s, s, s, s, s, -s, s, 0, 0, 1);
        face(s, -s, -s, s, s, -s, -s, s, -s, -s, -s, -s, 0, 0, -1);

        Mesh m; glGenVertexArrays(1, &m.vao); glGenBuffers(1, &m.vbo); glGenBuffers(1, &m.ebo);
        glBindVertexArray(m.vao);
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(v.size() * sizeof(V)), v.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(i.size() * sizeof(unsigned int)), i.data(), GL_STATIC_DRAW);
        setupAttribs();
        m.indexCount = (GLsizei)i.size();
        glBindVertexArray(0);
        return m;
    }

    Mesh makeSphere(int seg, int rings) {
        std::vector<V> v; std::vector<unsigned int> i;
        for (int y = 0; y <= rings; ++y) {
            for (int x = 0; x <= seg; ++x) {
                float u = (float)x / (float)seg;
                float vv = (float)y / (float)rings;
                float th = u * 2.0f * (float)M_PI;
                float ph = vv * (float)M_PI;
                float sx = std::sin(ph) * std::cos(th);
                float sy = std::cos(ph);
                float sz = std::sin(ph) * std::sin(th);
                v.push_back({ sx,sy,sz, sx,sy,sz });
            }
        }
        for (int y = 0; y < rings; ++y) {
            for (int x = 0; x < seg; ++x) {
                unsigned int a = y * (seg + 1) + x;
                unsigned int b = (y + 1) * (seg + 1) + x;
                i.insert(i.end(), { a,b,a + 1, b,b + 1,a + 1 });
            }
        }

        Mesh m; glGenVertexArrays(1, &m.vao); glGenBuffers(1, &m.vbo); glGenBuffers(1, &m.ebo);
        glBindVertexArray(m.vao);
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(v.size() * sizeof(V)), v.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(i.size() * sizeof(unsigned int)), i.data(), GL_STATIC_DRAW);
        setupAttribs();
        m.indexCount = (GLsizei)i.size();
        glBindVertexArray(0);
        return m;
    }

    Mesh makeCylinder(int seg) {
        std::vector<V> v; std::vector<unsigned int> i;
        for (int k = 0; k <= 1; ++k) {
            float y = (float)k;
            for (int s = 0; s <= seg; ++s) {
                float a = s * 2.0f * (float)M_PI / seg;
                float x = std::cos(a), z = std::sin(a);
                v.push_back({ x,y,z, x,0,z });
            }
        }
        for (int s = 0; s < seg; ++s) {
            unsigned int a = s, b = s + seg + 1, c = s + 1, d = s + seg + 2;
            i.insert(i.end(), { a,b,c, c,b,d });
        }

        Mesh m; glGenVertexArrays(1, &m.vao); glGenBuffers(1, &m.vbo); glGenBuffers(1, &m.ebo);
        glBindVertexArray(m.vao);
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(v.size() * sizeof(V)), v.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(i.size() * sizeof(unsigned int)), i.data(), GL_STATIC_DRAW);
        setupAttribs();
        m.indexCount = (GLsizei)i.size();
        glBindVertexArray(0);
        return m;
    }

    Mesh makeCone(int seg) {
        std::vector<V> v; std::vector<unsigned int> i;
        v.push_back({ 0.f,1.f,0.f, 0.f,1.f,0.f });
        for (int s = 0; s <= seg; ++s) {
            float a = s * 2.0f * (float)M_PI / seg;
            float x = std::cos(a), z = std::sin(a);
            float nx = x, nz = z, ny = 0.4f;
            float inv = 1.0f / std::sqrt(nx * nx + ny * ny + nz * nz);
            nx *= inv; ny *= inv; nz *= inv;
            v.push_back({ x,0.f,z, nx,ny,nz });
        }
        for (int s = 1; s <= seg; ++s) i.insert(i.end(), { 0u,(unsigned int)s,(unsigned int)(s + 1) });

        Mesh m; glGenVertexArrays(1, &m.vao); glGenBuffers(1, &m.vbo); glGenBuffers(1, &m.ebo);
        glBindVertexArray(m.vao);
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(v.size() * sizeof(V)), v.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(i.size() * sizeof(unsigned int)), i.data(), GL_STATIC_DRAW);
        setupAttribs();
        m.indexCount = (GLsizei)i.size();
        glBindVertexArray(0);
        return m;
    }

    void drawMesh(const Mesh& m) {
        glBindVertexArray(m.vao);
        glDrawElements(GL_TRIANGLES, m.indexCount, GL_UNSIGNED_INT, (void*)0);
        glBindVertexArray(0);
    }

    // ========================================================================
    // PROCEDURAL CHARACTER MODEL GENERATORS
    // ========================================================================

    Mesh createProceduralMouse() {
        std::cout << "    Building procedural mouse with body + head + ears...\n";
        std::vector<V> verts;
        std::vector<unsigned int> indices;

        const int seg = 16;
        const int rings = 12;
        unsigned int baseIdx = 0;

        // Body (stretched ellipsoid)
        for (int y = 0; y <= rings; ++y) {
            for (int x = 0; x <= seg; ++x) {
                float u = (float)x / (float)seg;
                float v = (float)y / (float)rings;
                float th = u * 2.0f * (float)M_PI;
                float ph = v * (float)M_PI;

                float sx = 0.4f * std::sin(ph) * std::cos(th);
                float sy = 0.5f * std::cos(ph);
                float sz = 0.6f * std::sin(ph) * std::sin(th);

                glm::vec3 n = glm::normalize(glm::vec3(sx, sy, sz));
                verts.push_back({ sx, sy, sz, n.x, n.y, n.z });
            }
        }

        for (int y = 0; y < rings; ++y) {
            for (int x = 0; x < seg; ++x) {
                unsigned int a = baseIdx + y * (seg + 1) + x;
                unsigned int b = baseIdx + (y + 1) * (seg + 1) + x;
                indices.insert(indices.end(), { a, b, a + 1, b, b + 1, a + 1 });
            }
        }

        baseIdx = (unsigned int)verts.size();

        // Head (small sphere in front)
        for (int y = 0; y <= rings; ++y) {
            for (int x = 0; x <= seg; ++x) {
                float u = (float)x / (float)seg;
                float v = (float)y / (float)rings;
                float th = u * 2.0f * (float)M_PI;
                float ph = v * (float)M_PI;

                float sx = 0.25f * std::sin(ph) * std::cos(th) + 0.4f;
                float sy = 0.25f * std::cos(ph) + 0.3f;
                float sz = 0.25f * std::sin(ph) * std::sin(th);

                glm::vec3 n = glm::normalize(glm::vec3(sx - 0.4f, sy - 0.3f, sz));
                verts.push_back({ sx, sy, sz, n.x, n.y, n.z });
            }
        }

        for (int y = 0; y < rings; ++y) {
            for (int x = 0; x < seg; ++x) {
                unsigned int a = baseIdx + y * (seg + 1) + x;
                unsigned int b = baseIdx + (y + 1) * (seg + 1) + x;
                indices.insert(indices.end(), { a, b, a + 1, b, b + 1, a + 1 });
            }
        }

        // Ears (two small triangles)
        baseIdx = (unsigned int)verts.size();
        verts.push_back({ 0.3f, 0.6f, -0.15f, 0, 1, 0 });
        verts.push_back({ 0.4f, 0.8f, -0.1f, 0, 1, 0 });
        verts.push_back({ 0.5f, 0.6f, -0.05f, 0, 1, 0 });
        indices.insert(indices.end(), { baseIdx, baseIdx + 1, baseIdx + 2 });

        baseIdx = (unsigned int)verts.size();
        verts.push_back({ 0.3f, 0.6f, 0.15f, 0, 1, 0 });
        verts.push_back({ 0.4f, 0.8f, 0.1f, 0, 1, 0 });
        verts.push_back({ 0.5f, 0.6f, 0.05f, 0, 1, 0 });
        indices.insert(indices.end(), { baseIdx, baseIdx + 1, baseIdx + 2 });

        Mesh m;
        glGenVertexArrays(1, &m.vao);
        glGenBuffers(1, &m.vbo);
        glGenBuffers(1, &m.ebo);
        glBindVertexArray(m.vao);
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(V)), verts.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(indices.size() * sizeof(unsigned int)), indices.data(), GL_STATIC_DRAW);
        setupAttribs();
        m.indexCount = (GLsizei)indices.size();
        glBindVertexArray(0);

        std::cout << "    ✓ Mouse created (" << verts.size() << " vertices, " << indices.size() / 3 << " triangles)\n";
        return m;
    }

    Mesh createProceduralCat() {
        std::cout << "    Building procedural cat with body + head + ears...\n";
        std::vector<V> verts;
        std::vector<unsigned int> indices;

        const int seg = 16;
        const int rings = 12;
        unsigned int baseIdx = 0;

        // Body (larger ellipsoid)
        for (int y = 0; y <= rings; ++y) {
            for (int x = 0; x <= seg; ++x) {
                float u = (float)x / (float)seg;
                float v = (float)y / (float)rings;
                float th = u * 2.0f * (float)M_PI;
                float ph = v * (float)M_PI;

                float sx = 0.5f * std::sin(ph) * std::cos(th);
                float sy = 0.6f * std::cos(ph);
                float sz = 0.7f * std::sin(ph) * std::sin(th);

                glm::vec3 n = glm::normalize(glm::vec3(sx, sy, sz));
                verts.push_back({ sx, sy, sz, n.x, n.y, n.z });
            }
        }

        for (int y = 0; y < rings; ++y) {
            for (int x = 0; x < seg; ++x) {
                unsigned int a = baseIdx + y * (seg + 1) + x;
                unsigned int b = baseIdx + (y + 1) * (seg + 1) + x;
                indices.insert(indices.end(), { a, b, a + 1, b, b + 1, a + 1 });
            }
        }

        baseIdx = (unsigned int)verts.size();

        // Head
        for (int y = 0; y <= rings; ++y) {
            for (int x = 0; x <= seg; ++x) {
                float u = (float)x / (float)seg;
                float v = (float)y / (float)rings;
                float th = u * 2.0f * (float)M_PI;
                float ph = v * (float)M_PI;

                float sx = 0.3f * std::sin(ph) * std::cos(th) + 0.5f;
                float sy = 0.3f * std::cos(ph) + 0.4f;
                float sz = 0.3f * std::sin(ph) * std::sin(th);

                glm::vec3 n = glm::normalize(glm::vec3(sx - 0.5f, sy - 0.4f, sz));
                verts.push_back({ sx, sy, sz, n.x, n.y, n.z });
            }
        }

        for (int y = 0; y < rings; ++y) {
            for (int x = 0; x < seg; ++x) {
                unsigned int a = baseIdx + y * (seg + 1) + x;
                unsigned int b = baseIdx + (y + 1) * (seg + 1) + x;
                indices.insert(indices.end(), { a, b, a + 1, b, b + 1, a + 1 });
            }
        }

        // Pointed ears
        baseIdx = (unsigned int)verts.size();
        verts.push_back({ 0.4f, 0.7f, -0.2f, 0, 1, 0 });
        verts.push_back({ 0.5f, 0.95f, -0.15f, 0, 1, 0 });
        verts.push_back({ 0.6f, 0.7f, -0.1f, 0, 1, 0 });
        indices.insert(indices.end(), { baseIdx, baseIdx + 1, baseIdx + 2 });

        baseIdx = (unsigned int)verts.size();
        verts.push_back({ 0.4f, 0.7f, 0.2f, 0, 1, 0 });
        verts.push_back({ 0.5f, 0.95f, 0.15f, 0, 1, 0 });
        verts.push_back({ 0.6f, 0.7f, 0.1f, 0, 1, 0 });
        indices.insert(indices.end(), { baseIdx, baseIdx + 1, baseIdx + 2 });

        Mesh m;
        glGenVertexArrays(1, &m.vao);
        glGenBuffers(1, &m.vbo);
        glGenBuffers(1, &m.ebo);
        glBindVertexArray(m.vao);
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(V)), verts.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(indices.size() * sizeof(unsigned int)), indices.data(), GL_STATIC_DRAW);
        setupAttribs();
        m.indexCount = (GLsizei)indices.size();
        glBindVertexArray(0);

        std::cout << "    ✓ Cat created (" << verts.size() << " vertices, " << indices.size() / 3 << " triangles)\n";
        return m;
    }

    Mesh createProceduralCheese() {
        std::cout << "    Building procedural cheese wedge...\n";
        std::vector<V> verts;
        std::vector<unsigned int> indices;

        // Triangular prism (wedge shape)
        glm::vec3 v0(-0.3f, 0, -0.2f);
        glm::vec3 v1(0.3f, 0, -0.2f);
        glm::vec3 v2(0.f, 0.4f, -0.2f);
        glm::vec3 v3(-0.3f, 0, 0.2f);
        glm::vec3 v4(0.3f, 0, 0.2f);
        glm::vec3 v5(0.f, 0.4f, 0.2f);

        unsigned int base = 0;

        // Front face
        glm::vec3 n1 = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        verts.push_back({ v0.x,v0.y,v0.z, n1.x,n1.y,n1.z });
        verts.push_back({ v1.x,v1.y,v1.z, n1.x,n1.y,n1.z });
        verts.push_back({ v2.x,v2.y,v2.z, n1.x,n1.y,n1.z });
        indices.insert(indices.end(), { base, base + 1, base + 2 });
        base += 3;

        // Back face
        glm::vec3 n2 = glm::normalize(glm::cross(v4 - v3, v5 - v3));
        verts.push_back({ v3.x,v3.y,v3.z, n2.x,n2.y,n2.z });
        verts.push_back({ v4.x,v4.y,v4.z, n2.x,n2.y,n2.z });
        verts.push_back({ v5.x,v5.y,v5.z, n2.x,n2.y,n2.z });
        indices.insert(indices.end(), { base, base + 2, base + 1 });
        base += 3;

        // Bottom
        verts.push_back({ v0.x,v0.y,v0.z, 0,-1,0 });
        verts.push_back({ v3.x,v3.y,v3.z, 0,-1,0 });
        verts.push_back({ v4.x,v4.y,v4.z, 0,-1,0 });
        verts.push_back({ v1.x,v1.y,v1.z, 0,-1,0 });
        indices.insert(indices.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });
        base += 4;

        // Left slant
        glm::vec3 n3 = glm::normalize(glm::cross(v3 - v0, v2 - v0));
        verts.push_back({ v0.x,v0.y,v0.z, n3.x,n3.y,n3.z });
        verts.push_back({ v2.x,v2.y,v2.z, n3.x,n3.y,n3.z });
        verts.push_back({ v5.x,v5.y,v5.z, n3.x,n3.y,n3.z });
        verts.push_back({ v3.x,v3.y,v3.z, n3.x,n3.y,n3.z });
        indices.insert(indices.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });
        base += 4;

        // Right slant
        glm::vec3 n4 = glm::normalize(glm::cross(v2 - v1, v4 - v1));
        verts.push_back({ v1.x,v1.y,v1.z, n4.x,n4.y,n4.z });
        verts.push_back({ v4.x,v4.y,v4.z, n4.x,n4.y,n4.z });
        verts.push_back({ v5.x,v5.y,v5.z, n4.x,n4.y,n4.z });
        verts.push_back({ v2.x,v2.y,v2.z, n4.x,n4.y,n4.z });
        indices.insert(indices.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });

        Mesh m;
        glGenVertexArrays(1, &m.vao);
        glGenBuffers(1, &m.vbo);
        glGenBuffers(1, &m.ebo);
        glBindVertexArray(m.vao);
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(V)), verts.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(indices.size() * sizeof(unsigned int)), indices.data(), GL_STATIC_DRAW);
        setupAttribs();
        m.indexCount = (GLsizei)indices.size();
        glBindVertexArray(0);

        std::cout << "    ✓ Cheese created (" << verts.size() << " vertices, " << indices.size() / 3 << " triangles)\n";
        return m;
    }

    // ========================================================================
    // OBJ LOADER with fallback to procedural models
    // ========================================================================

    Mesh loadOBJ(const std::string& path) {
        std::ifstream file(path);

        // If file doesn't exist, create procedural model based on filename
        if (!file) {
            std::cout << "  OBJ file not found: " << path << "\n";

            // Check which model is needed
            if (path.find("mouse") != std::string::npos) {
                std::cout << "  → Creating procedural MOUSE model\n";
                return createProceduralMouse();
            }
            else if (path.find("cat") != std::string::npos) {
                std::cout << "  → Creating procedural CAT model\n";
                return createProceduralCat();
            }
            else if (path.find("cheese") != std::string::npos) {
                std::cout << "  → Creating procedural CHEESE model\n";
                return createProceduralCheese();
            }
            else {
                std::cout << "  → Using sphere fallback\n";
                return makeSphere();
            }
        }

        // Standard OBJ loading code
        std::vector<glm::vec3> positions;
        std::vector<unsigned int> indices;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::istringstream iss(line);
            char c;
            if (line[0] == 'v' && (line[1] == ' ' || line[1] == '\t')) {
                iss >> c;
                float x, y, z;
                iss >> x >> y >> z;
                positions.emplace_back(x, y, z);
            }
            else if (line[0] == 'f' && (line[1] == ' ' || line[1] == '\t')) {
                iss >> c;
                std::string token;
                std::vector<unsigned int> face;
                while (iss >> token) {
                    size_t slash = token.find('/');
                    unsigned int vIndex = 0;
                    if (slash == std::string::npos)
                        vIndex = static_cast<unsigned int>(std::stoul(token));
                    else
                        vIndex = static_cast<unsigned int>(std::stoul(token.substr(0, slash)));
                    if (vIndex == 0) continue;
                    face.push_back(vIndex - 1);
                }
                if (face.size() >= 3) {
                    for (size_t i = 1; i + 1 < face.size(); ++i) {
                        indices.push_back(face[0]);
                        indices.push_back(face[i]);
                        indices.push_back(face[i + 1]);
                    }
                }
            }
        }

        if (positions.empty() || indices.empty()) {
            std::cerr << "  → OBJ has no geometry, using sphere fallback\n";
            return makeSphere();
        }

        // Compute per-vertex normals
        std::vector<glm::vec3> normals(positions.size(), glm::vec3(0));
        for (size_t i = 0; i + 2 < indices.size(); i += 3) {
            unsigned int i0 = indices[i];
            unsigned int i1 = indices[i + 1];
            unsigned int i2 = indices[i + 2];
            glm::vec3 p0 = positions[i0];
            glm::vec3 p1 = positions[i1];
            glm::vec3 p2 = positions[i2];
            glm::vec3 n = glm::normalize(glm::cross(p1 - p0, p2 - p0));
            if (glm::any(glm::isnan(n))) n = glm::vec3(0, 1, 0);
            normals[i0] += n;
            normals[i1] += n;
            normals[i2] += n;
        }
        for (auto& n : normals) {
            if (glm::length(n) > 1e-6f) n = glm::normalize(n);
            else n = glm::vec3(0, 1, 0);
        }

        std::vector<V> verts;
        verts.reserve(positions.size());
        for (size_t i = 0; i < positions.size(); ++i) {
            const auto& p = positions[i];
            const auto& n = normals[i];
            verts.push_back({ p.x,p.y,p.z, n.x,n.y,n.z });
        }

        Mesh m;
        glGenVertexArrays(1, &m.vao);
        glGenBuffers(1, &m.vbo);
        glGenBuffers(1, &m.ebo);

        glBindVertexArray(m.vao);
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(V)), verts.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(indices.size() * sizeof(unsigned int)), indices.data(), GL_STATIC_DRAW);
        setupAttribs();
        m.indexCount = (GLsizei)indices.size();
        glBindVertexArray(0);

        std::cout << "  ✓ Loaded OBJ: " << path << " (" << positions.size() << " vertices)\n";
        return m;
    }

} // namespace game
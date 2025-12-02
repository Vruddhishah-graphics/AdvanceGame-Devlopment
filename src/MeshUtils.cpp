#include "game/MeshUtils.h"
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include "game/glm_minimal.h"

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

    static Mesh createMeshFromData(const std::vector<V>& verts, const std::vector<unsigned int>& indices) {
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

        return m;
    }

    static void addSphere(std::vector<V>& verts, std::vector<unsigned int>& indices,
        glm::vec3 center, float radius, int segments = 16, int rings = 12) {

        unsigned int baseIdx = (unsigned int)verts.size();

        for (int y = 0; y <= rings; ++y) {
            for (int x = 0; x <= segments; ++x) {
                float u = (float)x / (float)segments;
                float v = (float)y / (float)rings;
                float theta = u * 2.0f * (float)M_PI;
                float phi = v * (float)M_PI;

                float px = std::sin(phi) * std::cos(theta);
                float py = std::cos(phi);
                float pz = std::sin(phi) * std::sin(theta);

                glm::vec3 normal(px, py, pz);
                glm::vec3 pos = center + normal * radius;

                verts.push_back({ pos.x, pos.y, pos.z, normal.x, normal.y, normal.z });
            }
        }

        for (int y = 0; y < rings; ++y) {
            for (int x = 0; x < segments; ++x) {
                unsigned int a = baseIdx + y * (segments + 1) + x;
                unsigned int b = baseIdx + (y + 1) * (segments + 1) + x;
                indices.insert(indices.end(), { a, b, a + 1, b, b + 1, a + 1 });
            }
        }
    }

    static void addEllipsoid(std::vector<V>& verts, std::vector<unsigned int>& indices,
        glm::vec3 center, glm::vec3 radii, int segments = 16, int rings = 12) {

        unsigned int baseIdx = (unsigned int)verts.size();

        for (int y = 0; y <= rings; ++y) {
            for (int x = 0; x <= segments; ++x) {
                float u = (float)x / (float)segments;
                float v = (float)y / (float)rings;
                float theta = u * 2.0f * (float)M_PI;
                float phi = v * (float)M_PI;

                float px = std::sin(phi) * std::cos(theta);
                float py = std::cos(phi);
                float pz = std::sin(phi) * std::sin(theta);

                glm::vec3 normal = glm::normalize(glm::vec3(px / radii.x, py / radii.y, pz / radii.z));
                glm::vec3 pos = center + glm::vec3(px * radii.x, py * radii.y, pz * radii.z);

                verts.push_back({ pos.x, pos.y, pos.z, normal.x, normal.y, normal.z });
            }
        }

        for (int y = 0; y < rings; ++y) {
            for (int x = 0; x < segments; ++x) {
                unsigned int a = baseIdx + y * (segments + 1) + x;
                unsigned int b = baseIdx + (y + 1) * (segments + 1) + x;
                indices.insert(indices.end(), { a, b, a + 1, b, b + 1, a + 1 });
            }
        }
    }

    // Helper to add cone - DECLARED BEFORE USE
    static void addCone(std::vector<V>& verts, std::vector<unsigned int>& indices,
        glm::vec3 center, float radius, float height, int segments) {

        unsigned int baseIdx = (unsigned int)verts.size();

        // Tip
        verts.push_back({ center.x, center.y + height, center.z, 0, 1, 0 });

        // Base circle
        for (int i = 0; i <= segments; ++i) {
            float angle = (float)i / segments * 2.0f * (float)M_PI;
            float x = center.x + std::cos(angle) * radius;
            float z = center.z + std::sin(angle) * radius;

            glm::vec3 normal = glm::normalize(glm::vec3(std::cos(angle), 0.5f, std::sin(angle)));
            verts.push_back({ x, center.y, z, normal.x, normal.y, normal.z });
        }

        // Triangles
        for (int i = 0; i < segments; ++i) {
            indices.insert(indices.end(), {
                baseIdx,
                baseIdx + i + 1,
                baseIdx + i + 2
                });
        }
    }

    // DETAILED MOUSE MODEL
    Mesh createDetailedMouse() {
        std::cout << "  Creating DETAILED procedural mouse...\n";
        std::vector<V> verts;
        std::vector<unsigned int> indices;

        // Body (main torso) - rounded ellipsoid
        addEllipsoid(verts, indices, { 0.0f, 0.0f, 0.0f }, { 0.35f, 0.4f, 0.5f }, 20, 16);

        // Head (slightly offset forward and up)
        addSphere(verts, indices, { 0.3f, 0.25f, 0.0f }, 0.28f, 18, 14);

        // Snout/nose (small sphere at front)
        addSphere(verts, indices, { 0.52f, 0.2f, 0.0f }, 0.12f, 12, 10);

        // Big ears (thin ellipsoids)
        addEllipsoid(verts, indices, { 0.25f, 0.5f, -0.2f }, { 0.08f, 0.22f, 0.18f }, 12, 10);
        addEllipsoid(verts, indices, { 0.25f, 0.5f, 0.2f }, { 0.08f, 0.22f, 0.18f }, 12, 10);

        // Eyes (tiny spheres)
        addSphere(verts, indices, { 0.45f, 0.32f, -0.1f }, 0.06f, 8, 6);
        addSphere(verts, indices, { 0.45f, 0.32f, 0.1f }, 0.06f, 8, 6);

        // Hands (small spheres)
        addSphere(verts, indices, { 0.2f, -0.15f, -0.35f }, 0.1f, 10, 8);
        addSphere(verts, indices, { 0.2f, -0.15f, 0.35f }, 0.1f, 10, 8);

        // Feet (slightly larger)
        addEllipsoid(verts, indices, { -0.15f, -0.35f, -0.25f }, { 0.12f, 0.08f, 0.15f }, 10, 8);
        addEllipsoid(verts, indices, { -0.15f, -0.35f, 0.25f }, { 0.12f, 0.08f, 0.15f }, 10, 8);

        // Tail (series of small spheres)
        for (int i = 0; i < 8; ++i) {
            float t = (float)i / 7.0f;
            float x = -0.4f - t * 0.6f;
            float y = -0.1f + std::sin(t * 3.14159f) * 0.15f;
            float radius = 0.05f * (1.0f - t * 0.5f);
            addSphere(verts, indices, { x, y, 0.0f }, radius, 8, 6);
        }

        std::cout << "    Detailed mouse: " << verts.size() << " vertices, "
            << indices.size() / 3 << " triangles\n";

        return createMeshFromData(verts, indices);
    }

    // DETAILED CAT MODEL
    Mesh createDetailedCat() {
        std::cout << "  Creating DETAILED procedural cat...\n";
        std::vector<V> verts;
        std::vector<unsigned int> indices;

        // Body (larger, more elongated)
        addEllipsoid(verts, indices, { 0.0f, 0.0f, 0.0f }, { 0.45f, 0.5f, 0.65f }, 20, 16);

        // Head (rounder, more forward)
        addSphere(verts, indices, { 0.5f, 0.3f, 0.0f }, 0.35f, 18, 14);

        // Snout (cat muzzle)
        addEllipsoid(verts, indices, { 0.72f, 0.2f, 0.0f }, { 0.15f, 0.12f, 0.15f }, 12, 10);

        // Pointed ears (triangular-ish) - NOW addCone is declared
        addCone(verts, indices, { 0.45f, 0.65f, -0.18f }, 0.15f, 0.25f, 12);
        addCone(verts, indices, { 0.45f, 0.65f, 0.18f }, 0.15f, 0.25f, 12);

        // Eyes (larger than mouse)
        addSphere(verts, indices, { 0.65f, 0.38f, -0.14f }, 0.08f, 10, 8);
        addSphere(verts, indices, { 0.65f, 0.38f, 0.14f }, 0.08f, 10, 8);

        // Nose
        addSphere(verts, indices, { 0.82f, 0.18f, 0.0f }, 0.05f, 8, 6);

        // Front legs
        addEllipsoid(verts, indices, { 0.25f, -0.35f, -0.35f }, { 0.12f, 0.35f, 0.12f }, 10, 12);
        addEllipsoid(verts, indices, { 0.25f, -0.35f, 0.35f }, { 0.12f, 0.35f, 0.12f }, 10, 12);

        // Back legs (thicker)
        addEllipsoid(verts, indices, { -0.25f, -0.3f, -0.35f }, { 0.15f, 0.4f, 0.15f }, 10, 12);
        addEllipsoid(verts, indices, { -0.25f, -0.3f, 0.35f }, { 0.15f, 0.4f, 0.15f }, 10, 12);

        // Paws
        addSphere(verts, indices, { 0.25f, -0.65f, -0.35f }, 0.11f, 10, 8);
        addSphere(verts, indices, { 0.25f, -0.65f, 0.35f }, 0.11f, 10, 8);
        addSphere(verts, indices, { -0.25f, -0.65f, -0.35f }, 0.13f, 10, 8);
        addSphere(verts, indices, { -0.25f, -0.65f, 0.35f }, 0.13f, 10, 8);

        // Tail (curved, thicker than mouse)
        for (int i = 0; i < 10; ++i) {
            float t = (float)i / 9.0f;
            float x = -0.5f - t * 0.8f;
            float y = 0.0f + std::sin(t * 3.14159f * 1.5f) * 0.3f;
            float z = std::sin(t * 3.14159f) * 0.15f;
            float radius = 0.08f * (1.0f - t * 0.4f);
            addSphere(verts, indices, { x, y, z }, radius, 8, 6);
        }

        std::cout << "    Detailed cat: " << verts.size() << " vertices, "
            << indices.size() / 3 << " triangles\n";

        return createMeshFromData(verts, indices);
    }

    // DETAILED CHEESE MODEL
    Mesh createDetailedCheese() {
        std::cout << "  Creating DETAILED procedural cheese...\n";
        std::vector<V> verts;
        std::vector<unsigned int> indices;

        // Main cheese wedge body
        glm::vec3 v0(-0.35f, 0, -0.25f);
        glm::vec3 v1(0.35f, 0, -0.25f);
        glm::vec3 v2(0.0f, 0.5f, -0.25f);
        glm::vec3 v3(-0.35f, 0, 0.25f);
        glm::vec3 v4(0.35f, 0, 0.25f);
        glm::vec3 v5(0.0f, 0.5f, 0.25f);

        unsigned int base = (unsigned int)verts.size();

        // Front face
        glm::vec3 n1 = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        verts.push_back({ v0.x, v0.y, v0.z, n1.x, n1.y, n1.z });
        verts.push_back({ v1.x, v1.y, v1.z, n1.x, n1.y, n1.z });
        verts.push_back({ v2.x, v2.y, v2.z, n1.x, n1.y, n1.z });
        indices.insert(indices.end(), { base, base + 1, base + 2 });
        base += 3;

        // Back face
        glm::vec3 n2 = glm::normalize(glm::cross(v4 - v3, v5 - v3));
        verts.push_back({ v3.x, v3.y, v3.z, -n2.x, -n2.y, -n2.z });
        verts.push_back({ v5.x, v5.y, v5.z, -n2.x, -n2.y, -n2.z });
        verts.push_back({ v4.x, v4.y, v4.z, -n2.x, -n2.y, -n2.z });
        indices.insert(indices.end(), { base, base + 1, base + 2 });
        base += 3;

        // Bottom
        verts.push_back({ v0.x, v0.y, v0.z, 0, -1, 0 });
        verts.push_back({ v3.x, v3.y, v3.z, 0, -1, 0 });
        verts.push_back({ v4.x, v4.y, v4.z, 0, -1, 0 });
        verts.push_back({ v1.x, v1.y, v1.z, 0, -1, 0 });
        indices.insert(indices.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });
        base += 4;

        // Left slant
        glm::vec3 n3 = glm::normalize(glm::cross(v3 - v0, v2 - v0));
        verts.push_back({ v0.x, v0.y, v0.z, n3.x, n3.y, n3.z });
        verts.push_back({ v2.x, v2.y, v2.z, n3.x, n3.y, n3.z });
        verts.push_back({ v5.x, v5.y, v5.z, n3.x, n3.y, n3.z });
        verts.push_back({ v3.x, v3.y, v3.z, n3.x, n3.y, n3.z });
        indices.insert(indices.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });
        base += 4;

        // Right slant
        glm::vec3 n4 = glm::normalize(glm::cross(v2 - v1, v4 - v1));
        verts.push_back({ v1.x, v1.y, v1.z, n4.x, n4.y, n4.z });
        verts.push_back({ v4.x, v4.y, v4.z, n4.x, n4.y, n4.z });
        verts.push_back({ v5.x, v5.y, v5.z, n4.x, n4.y, n4.z });
        verts.push_back({ v2.x, v2.y, v2.z, n4.x, n4.y, n4.z });
        indices.insert(indices.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });
        base += 4;

        // Add cheese holes (spherical indentations)
        addSphere(verts, indices, { 0.1f, 0.25f, 0.0f }, 0.08f, 8, 6);
        addSphere(verts, indices, { -0.15f, 0.15f, 0.1f }, 0.06f, 8, 6);
        addSphere(verts, indices, { 0.2f, 0.35f, -0.15f }, 0.07f, 8, 6);

        std::cout << "    Detailed cheese: " << verts.size() << " vertices, "
            << indices.size() / 3 << " triangles\n";

        return createMeshFromData(verts, indices);
    }

    // Create quad for UI
    Mesh createQuad() {
        std::vector<V> verts = {
            { -0.5f, -0.5f, 0.0f,  0, 0, 1 },
            {  0.5f, -0.5f, 0.0f,  0, 0, 1 },
            {  0.5f,  0.5f, 0.0f,  0, 0, 1 },
            { -0.5f,  0.5f, 0.0f,  0, 0, 1 }
        };

        std::vector<unsigned int> indices = { 0, 1, 2, 0, 2, 3 };

        return createMeshFromData(verts, indices);
    }

    // Basic primitives
    Mesh makeBox() {
        std::vector<V> v;
        std::vector<unsigned int> i;

        auto face = [&](float ax, float ay, float az, float bx, float by, float bz,
            float cx, float cy, float cz, float dx, float dy, float dz,
            float nx, float ny, float nz) {
                unsigned int base = (unsigned int)v.size();
                v.push_back({ ax, ay, az, nx, ny, nz });
                v.push_back({ bx, by, bz, nx, ny, nz });
                v.push_back({ cx, cy, cz, nx, ny, nz });
                v.push_back({ dx, dy, dz, nx, ny, nz });
                i.insert(i.end(), { base, base + 1, base + 2, base + 2, base + 3, base });
            };

        float s = 0.5f;
        face(s, -s, -s, s, s, -s, s, s, s, s, -s, s, 1, 0, 0);
        face(-s, -s, s, -s, s, s, -s, s, -s, -s, -s, -s, -1, 0, 0);
        face(-s, s, -s, s, s, -s, s, s, s, -s, s, s, 0, 1, 0);
        face(-s, -s, s, s, -s, s, s, -s, -s, -s, -s, -s, 0, -1, 0);
        face(-s, -s, s, -s, s, s, s, s, s, s, -s, s, 0, 0, 1);
        face(s, -s, -s, s, s, -s, -s, s, -s, -s, -s, -s, 0, 0, -1);

        return createMeshFromData(v, i);
    }

    Mesh makeSphere(int seg, int rings) {
        std::vector<V> v;
        std::vector<unsigned int> i;

        for (int y = 0; y <= rings; ++y) {
            for (int x = 0; x <= seg; ++x) {
                float u = (float)x / (float)seg;
                float vv = (float)y / (float)rings;
                float th = u * 2.0f * (float)M_PI;
                float ph = vv * (float)M_PI;
                float sx = std::sin(ph) * std::cos(th);
                float sy = std::cos(ph);
                float sz = std::sin(ph) * std::sin(th);
                v.push_back({ sx, sy, sz, sx, sy, sz });
            }
        }

        for (int y = 0; y < rings; ++y) {
            for (int x = 0; x < seg; ++x) {
                unsigned int a = y * (seg + 1) + x;
                unsigned int b = (y + 1) * (seg + 1) + x;
                i.insert(i.end(), { a, b, a + 1, b, b + 1, a + 1 });
            }
        }

        return createMeshFromData(v, i);
    }

    Mesh makeCylinder(int seg) {
        std::vector<V> v;
        std::vector<unsigned int> i;

        for (int k = 0; k <= 1; ++k) {
            float y = (float)k;
            for (int s = 0; s <= seg; ++s) {
                float a = s * 2.0f * (float)M_PI / seg;
                float x = std::cos(a), z = std::sin(a);
                v.push_back({ x, y, z, x, 0, z });
            }
        }

        for (int s = 0; s < seg; ++s) {
            unsigned int a = s, b = s + seg + 1, c = s + 1, d = s + seg + 2;
            i.insert(i.end(), { a, b, c, c, b, d });
        }

        return createMeshFromData(v, i);
    }

    Mesh makeCone(int seg) {
        std::vector<V> v;
        std::vector<unsigned int> i;

        v.push_back({ 0.f, 1.f, 0.f, 0.f, 1.f, 0.f });

        for (int s = 0; s <= seg; ++s) {
            float a = s * 2.0f * (float)M_PI / seg;
            float x = std::cos(a), z = std::sin(a);
            float nx = x, nz = z, ny = 0.4f;
            float inv = 1.0f / std::sqrt(nx * nx + ny * ny + nz * nz);
            nx *= inv; ny *= inv; nz *= inv;
            v.push_back({ x, 0.f, z, nx, ny, nz });
        }

        for (int s = 1; s <= seg; ++s)
            i.insert(i.end(), { 0u, (unsigned int)s, (unsigned int)(s + 1) });

        return createMeshFromData(v, i);
    }

    void drawMesh(const Mesh& m) {
        glBindVertexArray(m.vao);
        glDrawElements(GL_TRIANGLES, m.indexCount, GL_UNSIGNED_INT, (void*)0);
        glBindVertexArray(0);
    }

    // OBJ loader with fallback
    Mesh loadOBJ(const std::string& path) {
        std::ifstream file(path);

        if (!file) {
            std::cout << "  OBJ not found: " << path << "\n";

            if (path.find("mouse") != std::string::npos) {
                return createDetailedMouse();
            }
            else if (path.find("cat") != std::string::npos) {
                return createDetailedCat();
            }
            else if (path.find("cheese") != std::string::npos) {
                return createDetailedCheese();
            }
            else {
                return makeSphere();
            }
        }

        // Standard OBJ loading...
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

        if (positions.empty()) return makeSphere();

        std::vector<glm::vec3> normals(positions.size(), glm::vec3(0));
        for (size_t i = 0; i + 2 < indices.size(); i += 3) {
            glm::vec3 n = glm::normalize(glm::cross(
                positions[indices[i + 1]] - positions[indices[i]],
                positions[indices[i + 2]] - positions[indices[i]]
            ));
            normals[indices[i]] += n;
            normals[indices[i + 1]] += n;
            normals[indices[i + 2]] += n;
        }

        std::vector<V> verts;
        for (size_t i = 0; i < positions.size(); ++i) {
            glm::vec3 n = glm::length(normals[i]) > 1e-6f ?
                glm::normalize(normals[i]) : glm::vec3(0, 1, 0);
            verts.push_back({ positions[i].x, positions[i].y, positions[i].z, n.x, n.y, n.z });
        }

        return createMeshFromData(verts, indices);
    }

} // namespace game
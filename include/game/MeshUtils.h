#pragma once
#include <GL/glew.h>
#include <string>

namespace game {

    struct Mesh {
        GLuint vao = 0, vbo = 0, ebo = 0;
        GLsizei indexCount = 0;
    };

    // Basic primitives
    Mesh makeBox();
    Mesh makeSphere(int seg = 32, int rings = 16);
    Mesh makeCylinder(int seg = 24);
    Mesh makeCone(int seg = 24);
    void drawMesh(const Mesh& m);

    // OBJ loader (with procedural fallbacks)
    Mesh loadOBJ(const std::string& path);

    // Procedural character models
    Mesh createProceduralMouse();
    Mesh createProceduralCat();
    Mesh createProceduralCheese();

} // namespace game
#pragma once
#include <GL/glew.h>

namespace game {

struct Mesh {
    GLuint vao = 0, vbo = 0, ebo = 0;
    GLsizei indexCount = 0;
};

Mesh makeBox();
Mesh makeSphere(int seg = 32, int rings = 16);
Mesh makeCylinder(int seg = 24);
Mesh makeCone(int seg = 24);
void  drawMesh(const Mesh& m);

} // namespace game

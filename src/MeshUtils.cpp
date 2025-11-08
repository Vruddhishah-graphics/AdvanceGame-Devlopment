#include "game/MeshUtils.h"
#include <vector>
#include <cmath>

#define _USE_MATH_DEFINES
#include <math.h>

struct V { float x,y,z, nx,ny,nz; };

namespace game {

static void setupAttribs() {
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void*)(3 * sizeof(float)));
}

Mesh makeBox() {
    std::vector<V> v; std::vector<unsigned int> i;
    auto face = [&](float ax,float ay,float az,float bx,float by,float bz,float cx,float cy,float cz,float dx,float dy,float dz,
                    float nx,float ny,float nz){
        unsigned int base = (unsigned int)v.size();
        v.push_back({ax,ay,az, nx,ny,nz});
        v.push_back({bx,by,bz, nx,ny,nz});
        v.push_back({cx,cy,cz, nx,ny,nz});
        v.push_back({dx,dy,dz, nx,ny,nz});
        i.insert(i.end(), {base+0u,base+1u,base+2u, base+2u,base+3u,base+0u});
    };
    float s = 0.5f;
    face( s,-s,-s,  s, s,-s,  s, s, s,  s,-s, s,  1,0,0);
    face(-s,-s, s, -s, s, s, -s, s,-s, -s,-s,-s, -1,0,0);
    face(-s, s,-s,  s, s,-s,  s, s, s, -s, s, s,  0,1,0);
    face(-s,-s, s,  s,-s, s,  s,-s,-s, -s,-s,-s, 0,-1,0);
    face(-s,-s, s, -s, s, s,  s, s, s,  s,-s, s,  0,0,1);
    face( s,-s,-s,  s, s,-s, -s, s,-s, -s,-s,-s,  0,0,-1);

    Mesh m; glGenVertexArrays(1,&m.vao); glGenBuffers(1,&m.vbo); glGenBuffers(1,&m.ebo);
    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(v.size()*sizeof(V)), v.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(i.size()*sizeof(unsigned int)), i.data(), GL_STATIC_DRAW);
    setupAttribs();
    m.indexCount = (GLsizei)i.size();
    glBindVertexArray(0);
    return m;
}

Mesh makeSphere(int seg, int rings) {
    std::vector<V> v; std::vector<unsigned int> i;
    for (int y=0; y<=rings; ++y) {
        for (int x=0; x<=seg; ++x) {
            float u = (float)x / (float)seg;
            float vv= (float)y / (float)rings;
            float th = u * 2.0f * (float)M_PI;
            float ph = vv* (float)M_PI;
            float sx = std::sin(ph)*std::cos(th);
            float sy = std::cos(ph);
            float sz = std::sin(ph)*std::sin(th);
            v.push_back({sx,sy,sz, sx,sy,sz});
        }
    }
    for (int y=0; y<rings; ++y) {
        for (int x=0; x<seg; ++x) {
            unsigned int a =  y    * (seg+1) + x;
            unsigned int b = (y+1) * (seg+1) + x;
            i.insert(i.end(), {a,b,a+1, b,b+1,a+1});
        }
    }

    Mesh m; glGenVertexArrays(1,&m.vao); glGenBuffers(1,&m.vbo); glGenBuffers(1,&m.ebo);
    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(v.size()*sizeof(V)), v.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(i.size()*sizeof(unsigned int)), i.data(), GL_STATIC_DRAW);
    setupAttribs();
    m.indexCount = (GLsizei)i.size();
    glBindVertexArray(0);
    return m;
}

Mesh makeCylinder(int seg) {
    std::vector<V> v; std::vector<unsigned int> i;
    for (int k=0; k<=1; ++k) {
        float y = (float)k;
        for (int s=0; s<=seg; ++s) {
            float a = s * 2.0f * (float)M_PI / seg;
            float x = std::cos(a), z = std::sin(a);
            v.push_back({x,y,z, x,0,z});
        }
    }
    for (int s=0; s<seg; ++s) {
        unsigned int a = s, b = s + seg + 1, c = s+1, d = s + seg + 2;
        i.insert(i.end(), {a,b,c, c,b,d});
    }

    Mesh m; glGenVertexArrays(1,&m.vao); glGenBuffers(1,&m.vbo); glGenBuffers(1,&m.ebo);
    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(v.size()*sizeof(V)), v.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(i.size()*sizeof(unsigned int)), i.data(), GL_STATIC_DRAW);
    setupAttribs();
    m.indexCount = (GLsizei)i.size();
    glBindVertexArray(0);
    return m;
}

Mesh makeCone(int seg) {
    std::vector<V> v; std::vector<unsigned int> i;
    v.push_back({0.f,1.f,0.f, 0.f,1.f,0.f});
    for (int s=0; s<=seg; ++s) {
        float a = s * 2.0f * (float)M_PI / seg;
        float x = std::cos(a), z = std::sin(a);
        float nx = x, nz = z, ny = 0.4f;
        float inv = 1.0f/std::sqrt(nx*nx + ny*ny + nz*nz);
        nx*=inv; ny*=inv; nz*=inv;
        v.push_back({x,0.f,z, nx,ny,nz});
    }
    for (int s=1; s<=seg; ++s) i.insert(i.end(), {0u,(unsigned int)s,(unsigned int)(s+1)});

    Mesh m; glGenVertexArrays(1,&m.vao); glGenBuffers(1,&m.vbo); glGenBuffers(1,&m.ebo);
    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(v.size()*sizeof(V)), v.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(i.size()*sizeof(unsigned int)), i.data(), GL_STATIC_DRAW);
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

} // namespace game

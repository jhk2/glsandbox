#include "shapes.h"

Mesh<GLubyte>* createQuad()
{
    std::vector<PTvert> verts;
    PTvert v;
    v.pos_ = fl3(0,0,0); v.tex_ = fl3(0,0,0); verts.push_back(v);
    v.pos_ = fl3(1,0,0); v.tex_ = fl3(1,0,0); verts.push_back(v);
    v.pos_ = fl3(1,1,0); v.tex_ = fl3(1,1,0); verts.push_back(v);
    v.pos_ = fl3(0,1,0); v.tex_ = fl3(0,1,0); verts.push_back(v);
    std::vector<GLubyte> inds;
    inds.push_back(0x0); inds.push_back(0x1); inds.push_back(0x2); inds.push_back(0x3);
    InterleavedMesh<PTvert, GLubyte> *quad = new InterleavedMesh<PTvert, GLubyte>(GL_QUADS);
    quad->addAttrib(0, AttributeInfoSpec<GLfloat>(3)).addAttrib(1, AttributeInfoSpec<GLfloat>(3));
    quad->addVerts(verts).addInds(inds).finalize();
    return quad;
}

Mesh<GLubyte>* createIcosahedron()
{
    std::vector<fl3> icoverts;
    icoverts.push_back(fl3(0, 0, 1.0f));
    icoverts.push_back(fl3(0.894f,  0.000f,  0.447f));
    icoverts.push_back(fl3(0.276f,  0.851f,  0.447f));
    icoverts.push_back(fl3(-0.724f,  0.526f,  0.447f));
    icoverts.push_back(fl3(-0.724f, -0.526f,  0.447f));
    icoverts.push_back(fl3(0.276f, -0.851f,  0.447f));
    icoverts.push_back(fl3(0.724f,  0.526f, -0.447f));
    icoverts.push_back(fl3(-0.276f,  0.851f, -0.447f));
    icoverts.push_back(fl3(-0.894f,  0.000f, -0.447f));
    icoverts.push_back(fl3(-0.276f, -0.851f, -0.447f));
    icoverts.push_back(fl3(0.724f, -0.526f, -0.447f));
    icoverts.push_back(fl3(0.000f,  0.000f, -1.000f));
    std::vector<GLubyte> iinds;
    iinds.push_back(0); iinds.push_back(1); iinds.push_back(2);
    iinds.push_back(0); iinds.push_back(2); iinds.push_back(3);
    iinds.push_back(0); iinds.push_back(3); iinds.push_back(4);
    iinds.push_back(0); iinds.push_back(4); iinds.push_back(5);
    iinds.push_back(0); iinds.push_back(5); iinds.push_back(1);
    iinds.push_back(7); iinds.push_back(6); iinds.push_back(11);
    iinds.push_back(8); iinds.push_back(7); iinds.push_back(11);
    iinds.push_back(9); iinds.push_back(8); iinds.push_back(11);
    iinds.push_back(10); iinds.push_back(9); iinds.push_back(11);
    iinds.push_back(6); iinds.push_back(10); iinds.push_back(11);
    iinds.push_back(6); iinds.push_back(2); iinds.push_back(1);
    iinds.push_back(7); iinds.push_back(3); iinds.push_back(2);
    iinds.push_back(8); iinds.push_back(4); iinds.push_back(3);
    iinds.push_back(9); iinds.push_back(5); iinds.push_back(4);
    iinds.push_back(10); iinds.push_back(1); iinds.push_back(5);
    iinds.push_back(6); iinds.push_back(7); iinds.push_back(2);
    iinds.push_back(7); iinds.push_back(8); iinds.push_back(3);
    iinds.push_back(8); iinds.push_back(9); iinds.push_back(4);
    iinds.push_back(9); iinds.push_back(10); iinds.push_back(5);
    iinds.push_back(10); iinds.push_back(6); iinds.push_back(1);
    InterleavedMesh<fl3, GLubyte> *ico = new InterleavedMesh<fl3, GLubyte>(GL_TRIANGLES);
    ico->addAttrib(0, AttributeInfoSpec<GLfloat>(3));
    ico->addVerts(icoverts).addInds(iinds).finalize();
    return ico;
}

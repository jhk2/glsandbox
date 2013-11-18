#ifndef SHAPES_H
#define SHAPES_H

#include "mesh.h"
#include "utils.h"
// utility functions for making common shapes

Mesh<GLubyte>* createQuad();
Mesh<GLubyte>* createIcosahedron(const bool patches = false);

#endif // SHAPES_H

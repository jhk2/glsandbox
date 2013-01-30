#include "objloader.h"
#include <stdio.h>

ObjLoader::ObjLoader(const char *filename)
{
	loadFile(filename);
}

ObjLoader::~ObjLoader()
{
	
}

bool ObjLoader::loadFile(const char *filename)
{
	loadMaterials(filename);
	loadGeometry(filename);
}

bool ObjLoader::loadMaterials(const char *filename)
{
	
}

bool ObjLoader::loadGeometry(const char *filename)
{
	FILE *pFile = fopen(filename, "r");
	
	if(!pFile) {
		return false;
	}
	
	char buf[512] = "";
	
	while (fscanf(pFile, "%s", buf) > 0) {
		if (strcmp(buf, "v") == 0) {
			fl3 v;
			fscanf(pFile, "%f %f %f", &v.x, &v.y, &v.z);
			verts_.push_back(v);
		} else if (strcmp(buf, "vt") == 0) {
			fl3 t;
			fscanf(pFile, "%f %f %f", &v.x, &v.y, &v.z);
			texs_.push_back(t);
		} else if (strcmp(buf, "vn") == 0) {
			fl3 n;
			fscanf(pFile, "%f %f %f", &v.x, &v.y, &v.z);
			norms_.push_back(n);
		} else if (strcmp(buf, "f") == 0) {
			
		}
	}
}
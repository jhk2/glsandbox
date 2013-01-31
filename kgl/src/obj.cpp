#include "obj.h"
#include <stdio.h>
#include <assert.h>

Obj::Obj(const char *filename) : currentcombo_(0)
{
	loadFile(filename);
}

Obj::~Obj()
{
	
}

void Obj::draw()
{
	
}

bool Obj::loadFile(const char *filename)
{
	printf("trying to load obj file %s\n", filename); fflush(stdout);
	FILE *pFile = fopen(filename, "r");
	
	if(!pFile) {
		printf("failed to open obj file %s\n", filename); fflush(stdout);
		return false;
	}
	
	char line[1024] = "";
	char buf[512] = "";
	ObjMaterial *currentmat = 0;
	std::string meshname = "";
	
	// map for storing v/t/n triples versus index buffer
	std::map<int3, unsigned int> triples;
	fpos_t lastread;
	fgetpos(pFile, &lastread);
	while (fscanf(pFile, "%s", buf) > 0) {
		if (strcmp(buf, "mtllib") == 0) {
			// need to load the material file if we have not already
			fscanf(pFile, "%s", buf);
			std::string mtlfile (buf);
			if (mtlfiles_.find(mtlfile) == mtlfiles_.end()) {
				// add the path to the obj file to the name
				std::string fullpath (filename);
				size_t slashindex = fullpath.find_last_of("/");
				if (slashindex != std::string::npos) {
					mtlfile = fullpath.substr(0, slashindex + 1) + mtlfile;
				}
				// insert a new material
				loadMaterials(mtlfile.c_str());
			} else {
				printf("found undefined mtlfile %s, which is bad, but continuing\n", buf); fflush(stdout);
			}
		} else if (strcmp(buf, "usemtl") == 0) {
			fscanf(pFile, "%s", buf);
			std::string mtlfile (buf);
			currentmat = materials_[mtlfile];
			assert(currentmat != 0);
		} else if (strcmp(buf, "g") == 0) {
			fscanf(pFile, "%s", buf);
			meshname = buf;
		} else if (strcmp(buf, "v") == 0) {
			fl3 v;
			fscanf(pFile, "%f %f %f", &v.x, &v.y, &v.z);
			verts_.push_back(v);
		} else if (strcmp(buf, "vt") == 0) {
			fl3 t;
			fscanf(pFile, "%f %f %f", &t.x, &t.y, &t.z);
			texs_.push_back(t);
		} else if (strcmp(buf, "vn") == 0) {
			fl3 n;
			fscanf(pFile, "%f %f %f", &n.x, &n.y, &n.z);
			norms_.push_back(n);
		} else if (strcmp(buf, "f") == 0) {
			// move it back to before the f
			fsetpos(pFile, &lastread);
			// we make the assumption that all faces for a particular mesh are consecutively located in the file
			// get the face indices
			ObjMesh *currentmesh = 0;
			if (texs_.size() == 0 && norms_.size() == 0) {
				// we only have positions
				currentmesh = createPMesh(pFile);
			} else if (texs_.size() == 0) {
				// we have positions and normals
				currentmesh = createPNMesh(pFile);
			} else if (norms_.size() == 0) {
				// we have positions and texcoords
				currentmesh = createPTMesh(pFile);
			} else {
				// we have all 3
				currentmesh = createPTNMesh(pFile);
			}
			// put it in the map
			if(meshname.length() == 0) {
				meshname = "default";
			}
			if(meshes_.find(meshname) == meshes_.end()) {
				meshes_[meshname] = std::pair<ObjMesh*,ObjMaterial*>(currentmesh, currentmat);
			} else {
				printf("tried to insert mesh which already existed with name %s\n", meshname.c_str());
			}
		}
		fgetpos(pFile, &lastread);
	}
	return true;
}

bool Obj::loadMaterials(const char *filename)
{
	printf("trying to load mtl file %s\n", filename); fflush(stdout);
	FILE *pFile = fopen(filename, "r");
	if(!pFile) {
		printf("couldn't open mtlfile %s\n", filename); fflush(stdout);
		return false;
	}
	
	ObjMaterial *currentmat = 0;
	
	char buf[512] = "";
	while (fscanf(pFile, "%s", buf) > 0) {
		if (strcmp(buf, "newmtl") == 0) {
			fscanf(pFile, "%s", buf);
			ObjMaterial *newmat = new ObjMaterial();
			newmat->name = std::string(buf);
			materials_[newmat->name] = newmat;
			currentmat = newmat;
		} else if (strcmp(buf, "Ns") == 0) {
			float val;
			fscanf(pFile, "%f", &val);
			currentmat->Ns = val;
		} else if (strcmp(buf, "Ni") == 0) {
			float val;
			fscanf(pFile, "%f", &val);
			currentmat->Ni = val;
		} else if (strcmp(buf, "d") == 0 || strcmp(buf, "Tr") == 0) {
			float val;
			fscanf(pFile, "%f", &val);
			// for d/tf, might have another value already so take the max
			currentmat->d = max(currentmat->d, val);
		} else if (strcmp(buf, "illum") == 0) {
			unsigned int val;
			fscanf(pFile, "%u", &val);
			currentmat->illum = val;
		} else if (strcmp(buf, "Ka") == 0) {
			fl3 val;
			fscanf(pFile, "%f %f %f", &val.x, &val.y, &val.z);
			currentmat->Ka = val;
		} else if (strcmp(buf, "Kd") == 0) {
			fl3 val;
			fscanf(pFile, "%f %f %f", &val.x, &val.y, &val.z);
			currentmat->Kd = val;
		} else if (strcmp(buf, "Ks") == 0) {
			fl3 val;
			fscanf(pFile, "%f %f %f", &val.x, &val.y, &val.z);
			currentmat->Ks = val;
		} else if (strcmp(buf, "Ke") == 0) {
			fl3 val;
			fscanf(pFile, "%f %f %f", &val.x, &val.y, &val.z);
			currentmat->Ke = val;
		} else if (strcmp(buf, "map_Ka") == 0) {
			fscanf(pFile, "%s", buf);
			std::string texpath (buf);
			if (textures_.find(texpath) == textures_.end()) {
				// insert a new texture
				textures_[texpath] = Texture(buf);
			}
			currentmat->map_Ka = textures_[texpath];
		} else if (strcmp(buf, "map_Kd") == 0) {
			fscanf(pFile, "%s", buf);
			std::string texpath (buf);
			if (textures_.find(texpath) == textures_.end()) {
				// insert a new texture
				textures_[texpath] = Texture(buf);
			}
			currentmat->map_Kd = textures_[texpath];
		} else if (strcmp(buf, "map_Ks") == 0) {
			fscanf(pFile, "%s", buf);
			std::string texpath (buf);
			if (textures_.find(texpath) == textures_.end()) {
				// insert a new texture
				textures_[texpath] = Texture(buf);
			}
			currentmat->map_Ks = textures_[texpath];
		}
	}
	return true;	
}

Obj::ObjMesh* Obj::createPTNMesh(FILE *file)
{
	printf("creating ptn mesh\n"); fflush(stdout);
	unsigned int read = 0;
	Mesh<PTNvert, GLuint> *mesh = new Mesh<PTNvert, GLuint>(GL_TRIANGLES);
	// 3 vertex attributes with 3 floats each
	mesh->addAttrib(0, AttributeInfoSpec<GLfloat>(3)).addAttrib(1, AttributeInfoSpec<GLfloat>(3)).addAttrib(2, AttributeInfoSpec<GLfloat>(3));
	fpos_t lastpos;
	char buf[512] = "";
	fgetpos(file, &lastpos);
	while (fscanf(file, "%s", buf) > 0) {
		if (strcmp(buf, "f") != 0) {
			// we hit something other than "f"
			printf("hit a non f character %s after %u faces\n", buf, read); fflush(stdout);
			fsetpos(file, &lastpos);
			break;
		} else {
			// read the face
			int3 v[3];
			fscanf(file, "%u/%u/%u %u/%u/%u %u/%u/%u", &v[0].x, &v[0].y, &v[0].z, &v[1].x, &v[1].y, &v[1].z, &v[2].x, &v[2].y, &v[2].z);
			// check if each of them is contained
			PTNvert vert;
			for (int i = 0; i < 3; i++) {
				int3 &cur = v[i];
				if (combos_.find(cur) == combos_.end()) {
					combos_[cur] = currentcombo_;
					inds_.push_back(currentcombo_);
					currentcombo_++;
					vert.pos_ = verts_[cur.x - 1];
					vert.tex_ = verts_[cur.y - 1];
					vert.norm_ = verts_[cur.z - 1];
					mesh->addVert(vert);
				} else {
					inds_.push_back(combos_[cur]);
				}
			}
			read++;
		}
		fgetpos(file, &lastpos);
	}
	// we've read all the faces, so make the mesh
	return new PTNMesh(mesh);
}

Obj::ObjMesh* Obj::createPTMesh(FILE *file)
{
	Mesh<PTvert, GLuint> *mesh = new Mesh<PTvert, GLuint>(GL_TRIANGLES);
	// 2 vertex attributes with 3 floats each
	mesh->addAttrib(0, AttributeInfoSpec<GLfloat>(3)).addAttrib(1, AttributeInfoSpec<GLfloat>(3));
	fpos_t lastpos;
	char buf[512] = "";
	fgetpos(file, &lastpos);
	while (fscanf(file, "%s", buf) > 0) {
		if (strcmp(buf, "f") != 0) {
			// we hit something other than "f"
			fsetpos(file, &lastpos);
			break;
		} else {
			// read the face
			int3 v[3];
			fscanf(file, "%u/%u %u/%u %u/%u", &v[0].x, &v[0].y, &v[1].x, &v[1].y, &v[2].x, &v[2].y);
			// check if each of them is contained
			PTvert vert;
			for (int i = 0; i < 3; i++) {
				int3 &cur = v[i];
				if (combos_.find(cur) == combos_.end()) {
					combos_[cur] = currentcombo_;
					inds_.push_back(currentcombo_);
					currentcombo_++;
					vert.pos_ = verts_[cur.x - 1];
					vert.tex_ = verts_[cur.y - 1];
					mesh->addVert(vert);
				} else {
					inds_.push_back(combos_[cur]);
				}
			}
		}
		fgetpos(file, &lastpos);
	}
	// we've read all the faces, so make the mesh
	return new PTMesh(mesh);
}

Obj::ObjMesh* Obj::createPNMesh(FILE *file)
{
	printf("creating pn mesh\n"); fflush(stdout);
	Mesh<PNvert, GLuint> *mesh = new Mesh<PNvert, GLuint>(GL_TRIANGLES);
	// 2 vertex attributes with 3 floats each
	mesh->addAttrib(0, AttributeInfoSpec<GLfloat>(3)).addAttrib(2, AttributeInfoSpec<GLfloat>(3));
	fpos_t lastpos;
	char buf[512] = "";
	fgetpos(file, &lastpos);
	while (fscanf(file, "%s", buf) > 0) {
		if (strcmp(buf, "f") != 0) {
			// we hit something other than "f"
			printf("hit something other than f, ending mesh\n"); fflush(stdout);
			fsetpos(file, &lastpos);
			break;
		} else {
			// read the face
			int3 v[3];
			fscanf(file, "%u//%u %u//%u %u//%u", &v[0].x, &v[0].y, &v[1].x, &v[1].y, &v[2].x, &v[2].y);
			// check if each of them is contained
			PNvert vert;
			for (int i = 0; i < 3; i++) {
				int3 &cur = v[i];
				if (combos_.find(cur) == combos_.end()) {
					combos_[cur] = currentcombo_;
					inds_.push_back(currentcombo_);
					currentcombo_++;
					vert.pos_ = verts_[cur.x - 1];
					vert.norm_ = verts_[cur.y - 1];
					mesh->addVert(vert);
				} else {
					inds_.push_back(combos_[cur]);
				}
			}
		}
		fgetpos(file, &lastpos);
	}
	// we've read all the faces, so make the mesh
	return new PNMesh(mesh);
}

Obj::ObjMesh* Obj::createPMesh(FILE *file)
{
	Mesh<fl3, GLuint> *mesh = new Mesh<fl3, GLuint>(GL_TRIANGLES);
	// 2 vertex attributes with 3 floats each
	mesh->addAttrib(0, AttributeInfoSpec<GLfloat>(3));
	fpos_t lastpos;
	char buf[512] = "";
	fgetpos(file, &lastpos);
	while (fscanf(file, "%s", buf) > 0) {
		if (strcmp(buf, "f") != 0) {
			// we hit something other than "f"
			fsetpos(file, &lastpos);
			break;
		} else {
			// read the face
			int3 v[3];
			fscanf(file, "%u/%u %u/%u %u/%u", &v[0].x, &v[1].x, &v[2].x);
			// check if each of them is contained
			fl3 vert;
			for (int i = 0; i < 3; i++) {
				int3 &cur = v[i];
				if (combos_.find(cur) == combos_.end()) {
					combos_[cur] = currentcombo_;
					inds_.push_back(currentcombo_);
					currentcombo_++;
					vert = verts_[cur.x - 1];
					mesh->addVert(vert);
				} else {
					inds_.push_back(combos_[cur]);
				}
			}
		}
		fgetpos(file, &lastpos);
	}
	// we've read all the faces, so make the mesh
	return new PMesh(mesh);
}

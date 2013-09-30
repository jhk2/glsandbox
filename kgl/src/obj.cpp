#include "obj.h"
#include <stdio.h>
#include <assert.h>
#include "shader.h"
#include "matrixstack.h"

Obj::Obj(const char *filename) : currentcombo_(0), min_(), max_()
{
	loadFile(filename);
}

Obj::~Obj()
{
	for (std::map<std::string, std::pair<ObjMesh *, ObjMaterial *>>::iterator iter = meshes_.begin(); iter != meshes_.end(); iter++) {
		delete iter->second.first;
	}
	for (std::map<std::string, ObjMaterial *>::iterator iter = materials_.begin(); iter != materials_.end(); iter++) {
		delete iter->second;
	}
	for (std::map<std::string, Texture *>::iterator iter = textures_.begin(); iter != textures_.end(); iter++) {
		delete iter->second;
	}
}

void Obj::draw(const Shader &shader) const
{
	// assume that if more than one shader stage needs the uniforms they will be bound to the same pipeline object
	//~ printf("drawing obj\n"); fflush(stdout);
	// go through all of the sub meshes in the map
    for (std::map<std::string, std::pair<ObjMesh *, ObjMaterial *>>::const_iterator iter = meshes_.begin(); iter != meshes_.end(); iter++) {
		//~ printf("drawing submesh %s\n", iter->first.c_str()); fflush(stdout);
		//~ printf("get material\n"); fflush(stdout);
		ObjMaterial &curmat = *(iter->second.second);
		
		/*
		// set material parameters as uniform values
		//~ printf("get uniform locations\n"); fflush(stdout);
		GLint Ns, Ni, Tr, Tf, illum, Ka, Kd, Ks, Ke, map_Ka, map_Kd, map_Ks;
		Ns = shader.getUniformLocation("Ns");
		Ni = shader.getUniformLocation("Ni");
		Tr = shader.getUniformLocation("Tr");
		Tf = shader.getUniformLocation("Tf");
		illum = shader.getUniformLocation("illum");
		Ka = shader.getUniformLocation("Ka");
		Kd = shader.getUniformLocation("Kd");
		Ks = shader.getUniformLocation("Ks");
		Ke = shader.getUniformLocation("Ke");
		map_Ka = shader.getUniformLocation("map_Ka");
		map_Kd = shader.getUniformLocation("map_Kd");
		map_Ks = shader.getUniformLocation("map_Ks");
		
		//~ printf("set uniform values\n"); fflush(stdout);
		shader.use();
		if (Ns != -1) {
			glUniform1f(Ns, curmat.Ns);
		}
		if (Ni != -1) {
			glUniform1f(Ni, curmat.Ni);
		}
		if (Tr != -1) {
			glUniform1f(Tr, curmat.Tr);
		}
		if (Tf != -1) {
			glUniform3fv(Tf, 1, &curmat.Tf.x);
		}
		if (illum != -1) {
			glUniform1ui(illum, curmat.illum);
		}
		if (Ka != -1) {
			glUniform3fv(Ka, 1, &curmat.Ka.x);
		}
		if (Kd != -1) {
			glUniform3fv(Kd, 1, &curmat.Kd.x);
		}
		if (Ks != -1) {
			glUniform3fv(Ks, 1, &curmat.Ks.x);
		}
		if (Ke != -1) {
			glUniform3fv(Ke, 1, &curmat.Ke.x);
		}
		if (map_Ka != -1) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, curmat.map_Ka->getID());
			glUniform1i(map_Ka, 0);
		}
		if (map_Kd != -1) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, curmat.map_Kd->getID());
			glUniform1i(map_Kd, 1);
		}
		if (map_Ks != -1) {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, curmat.map_Ks->getID());
			glUniform1i(map_Ks, 2);
		}
		*/

		// Bind uniform buffer with material parameters
		GLuint blockIndex = shader.getUniformBlockIndex("ObjMaterial");
		if (blockIndex != -1) {
			glBindBufferBase(GL_UNIFORM_BUFFER, blockIndex, curmat.ubo);
		}

		GLint map_Ka, map_Kd, map_Ks;
		map_Ka = shader.getUniformLocation("map_Ka");
		map_Kd = shader.getUniformLocation("map_Kd");
		map_Ks = shader.getUniformLocation("map_Ks");

		// bind textures to pre-determined locations
		if (map_Ka != -1) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, curmat.map_Ka->getID());
			glUniform1i(map_Ka, 0);
		}
		if (map_Kd != -1) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, curmat.map_Kd->getID());
			glUniform1i(map_Kd, 1);
		}
		if (map_Ks != -1) {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, curmat.map_Ks->getID());
			glUniform1i(map_Ks, 2);
		}

		//~ glActiveTexture(GL_TEXTURE0);
		//~ glBindTexture(GL_TEXTURE_2D, curmat.map_Ka->getID());
		//~ glActiveTexture(GL_TEXTURE1);
		//~ glBindTexture(GL_TEXTURE_2D, curmat.map_Kd->getID());
		//~ glActiveTexture(GL_TEXTURE2);
		//~ glBindTexture(GL_TEXTURE_2D, curmat.map_Ks->getID());
		
		// draw the actual mesh
		ObjMesh &curmesh = *(iter->second.first);
		curmesh.draw();
		
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		if (blockIndex != -1) {
			glBindBufferBase(GL_UNIFORM_BUFFER, blockIndex, 0);
		}
	}
}

void Obj::getBounds(fl3 &min, fl3 &max)
{
	min = min_;
	max = max_;
}

bool Obj::loadFile(const char *filename)
{
    //printf("trying to load obj file %s\n", filename); fflush(stdout);
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
            //printf("loading submesh %s\n", buf); fflush(stdout);
			meshname = buf;
		} else if (strcmp(buf, "v") == 0) {
			fl3 v;
			fscanf(pFile, "%f %f %f", &v.x, &v.y, &v.z);
			// update min/max
			min_.x = min(min_.x, v.x);
			min_.y = min(min_.y, v.y);
			min_.z = min(min_.z, v.z);
			max_.x = max(max_.x, v.x);
			max_.y = max(max_.y, v.y);
			max_.z = max(max_.z, v.z);
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
                //printf("create new PMesh\n"); fflush(stdout);
				currentmesh = createPMesh(pFile);
			} else if (texs_.size() == 0) {
				// we have positions and normals
				//printf("we've read %u verts and %u normals\n", verts_.size(), norms_.size()); fflush(stdout);
                //printf("create new PNMesh\n"); fflush(stdout);
				currentmesh = createPNMesh(pFile);
			} else if (norms_.size() == 0) {
				// we have positions and texcoords
                //printf("create new PTMesh\n"); fflush(stdout);
				currentmesh = createPTMesh(pFile);
			} else {
				// we have all 3
                //printf("create new PTNMesh\n"); fflush(stdout);
				currentmesh = createPTNMesh(pFile);
			}
			// put it in the map
			if(meshname.length() == 0) {
				meshname = "default";
			}
			if(meshes_.find(meshname) == meshes_.end()) {
                //printf("inserting new mesh with name %s\n", meshname.c_str()); fflush(stdout);
				meshes_[meshname] = std::pair<ObjMesh*,ObjMaterial*>(currentmesh, currentmat);
			} else {
				printf("tried to insert mesh which already existed with name %s\n", meshname.c_str()); fflush(stdout);
			}
		}
		fgetpos(pFile, &lastread);
	}
	// clear temporary stuff
	verts_.clear();
	texs_.clear();
	norms_.clear();
	inds_.clear();
	mtlfiles_.clear();
	combos_.clear();
	return true;
}

bool Obj::loadMaterials(const char *filename)
{
    //printf("trying to load mtl file %s\n", filename); fflush(stdout);
	FILE *pFile = fopen(filename, "r");
	if(!pFile) {
		printf("couldn't open mtlfile %s\n", filename); fflush(stdout);
		return false;
	}
	
	// get the directory
	std::string fullpath (filename);
	size_t slashindex = fullpath.find_last_of("/");
	std::string directory = "";
	if (slashindex != std::string::npos) {
		directory = fullpath.substr(0, slashindex + 1);
	}
	
	ObjMaterial *currentmat = 0;
	
	char buf[512] = "";
	while (fscanf(pFile, "%s", buf) > 0) {
		//printf("buf contents %s\n", buf); fflush(stdout);
		if (strcmp(buf, "newmtl") == 0) {
			fscanf(pFile, "%s", buf);
			ObjMaterial *newmat = new ObjMaterial();
			newmat->name = std::string(buf);
			materials_[newmat->name] = newmat;
			currentmat = newmat;
		} else if (strcmp(buf, "Ns") == 0) {
			float val;
			fscanf(pFile, "%f", &val);
			currentmat->ublock.Ns = val;
		} else if (strcmp(buf, "Ni") == 0) {
			float val;
			fscanf(pFile, "%f", &val);
			currentmat->ublock.Ni = val;
		} else if (strcmp(buf, "d") == 0 || strcmp(buf, "Tr") == 0) {
			float val;
			fscanf(pFile, "%f", &val);
			// for d/tf, might have another value already so take the max
			currentmat->ublock.d = max(currentmat->ublock.d, val);
		} else if (strcmp(buf, "illum") == 0) {
			unsigned int val;
			fscanf(pFile, "%u", &val);
			currentmat->ublock.illum = val;
		} else if (strcmp(buf, "Ka") == 0) {
			fl3 val;
			fscanf(pFile, "%f %f %f", &val.x, &val.y, &val.z);
			currentmat->ublock.Ka = val;
		} else if (strcmp(buf, "Kd") == 0) {
			fl3 val;
			fscanf(pFile, "%f %f %f", &val.x, &val.y, &val.z);
			currentmat->ublock.Kd = val;
		} else if (strcmp(buf, "Ks") == 0) {
			fl3 val;
			fscanf(pFile, "%f %f %f", &val.x, &val.y, &val.z);
			currentmat->ublock.Ks = val;
		} else if (strcmp(buf, "Ke") == 0) {
			fl3 val;
			fscanf(pFile, "%f %f %f", &val.x, &val.y, &val.z);
			currentmat->ublock.Ke = val;
		} else if (strcmp(buf, "map_Ka") == 0) {
			fscanf(pFile, "%s", buf);
			std::string texpath (buf);
			std::string fulltexpath = directory + texpath;
			if (textures_.find(texpath) == textures_.end()) {
				// insert a new texture
				textures_[texpath] = new Texture(fulltexpath.c_str());
			}
			currentmat->map_Ka = textures_[texpath];
		} else if (strcmp(buf, "map_Kd") == 0) {
			fscanf(pFile, "%s", buf);
			std::string texpath (buf);
			std::string fulltexpath = directory + buf;
			if (textures_.find(texpath) == textures_.end()) {
				// insert a new texture
				textures_[texpath] = new Texture(fulltexpath.c_str());
			}
			currentmat->map_Kd = textures_[texpath];
		} else if (strcmp(buf, "map_Ks") == 0) {
			fscanf(pFile, "%s", buf);
			std::string texpath (buf);
			std::string fulltexpath = directory + buf;
			if (textures_.find(texpath) == textures_.end()) {
				// insert a new texture
				textures_[texpath] = new Texture(fulltexpath.c_str());
			}
			currentmat->map_Ks = textures_[texpath];
		}
	}
	
	for (std::map<std::string, ObjMaterial *>::iterator it = materials_.begin(); it != materials_.end(); it++) {
		ObjMaterial *mat = it->second;
		// make a uniform buffer for these properties
		GLuint &ubo = mat->ubo;
		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(ObjMaterial::ObjUniformBlock), &currentmat->ublock, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	return true;
}

Obj::ObjMesh* Obj::createPTNMesh(FILE *file)
{
	//~ printf("creating ptn mesh\n"); fflush(stdout);
	unsigned int read = 0;
	InterleavedMesh<PTNvert, GLuint> *mesh = new InterleavedMesh<PTNvert, GLuint>(GL_TRIANGLES);
	currentcombo_ = 0;
	combos_.clear();
	// 3 vertex attributes with 3 floats each
	mesh->addAttrib(0, AttributeInfoSpec<GLfloat>(3)).addAttrib(1, AttributeInfoSpec<GLfloat>(3)).addAttrib(2, AttributeInfoSpec<GLfloat>(3));
	fpos_t lastpos;
	char buf[512] = "";
	fgetpos(file, &lastpos);
	while (fscanf(file, "%s", buf) > 0) {
		if (strcmp(buf, "f") != 0) {
			// we hit something other than "f"
			//~ printf("hit a non f character %s after %u faces\n", buf, read); fflush(stdout);
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
					//~ printf("making new vertex entry for combo %u/%u/%u index %u\n", cur.x, cur.y, cur.z, currentcombo_); fflush(stdout);
					combos_[cur] = currentcombo_;
					//inds_.push_back(currentcombo_);
					mesh->addInd(currentcombo_);
					currentcombo_++;
					vert.pos_ = verts_[cur.x - 1];
					vert.tex_ = texs_[cur.y - 1];
					vert.norm_ = norms_[cur.z - 1];
					//~ printf("adding new vertex combo index %u pos %f,%f,%f (%u) tex %f,%f,%f (%u)\n", currentcombo_, vert.pos_.x, vert.pos_.y, vert.pos_.z, cur.x-1, vert.tex_.x, vert.tex_.y, vert.tex_.z, cur.y-1); fflush(stdout);
					mesh->addVert(vert);
				} else {
					//~ printf("found existing entry for combo %u/%u/%u index %u\n", cur.x, cur.y, cur.z, combos_[cur]); fflush(stdout);
					//inds_.push_back(combos_[cur]);
					mesh->addInd(combos_[cur]);
				}
			}
			read++;
		}
		fgetpos(file, &lastpos);
	}
	mesh->finalize();
	// we've read all the faces, so make the mesh
	return new ObjMesh(mesh);
}

Obj::ObjMesh* Obj::createPTMesh(FILE *file)
{
	unsigned int read = 0;
	currentcombo_ = 0;
	combos_.clear();
	InterleavedMesh<PTvert, GLuint> *mesh = new InterleavedMesh<PTvert, GLuint>(GL_TRIANGLES);
	// 2 vertex attributes with 3 floats each
	mesh->addAttrib(0, AttributeInfoSpec<GLfloat>(3)).addAttrib(1, AttributeInfoSpec<GLfloat>(3));
	fpos_t lastpos;
	char buf[512] = "";
	fgetpos(file, &lastpos);
	while (fscanf(file, "%s", buf) > 0) {
		//~ printf("buf contents: %s\n", buf); fflush(stdout);
		if (strcmp(buf, "f") != 0) {
			// we hit something other than "f"
			//~ printf("hit something other than f: %s\n", buf); fflush(stdout);
			fsetpos(file, &lastpos);
			break;
		} else {
			// read the face
			int3 v[3];
			unsigned int scanned = fscanf(file, "%u/%u %u/%u %u/%u", &v[0].x, &v[0].y, &v[1].x, &v[1].y, &v[2].x, &v[2].y);
			//~ if(scanned != 6) {
				//~ printf("scanned less than 6 items: %u/%u %u/%u %u/%u\n", v[0].x, v[0].y, v[1].x, v[1].y, v[2].x, v[2].y);
				//~ fflush(stdout);
			//~ }
			// check if each of them is contained
			PTvert vert;
			for (int i = 0; i < 3; i++) {
				int3 &cur = v[i];
				if (combos_.find(cur) == combos_.end()) {
					combos_[cur] = currentcombo_;
					//inds_.push_back(currentcombo_);
					mesh->addInd(currentcombo_);
					currentcombo_++;
					vert.pos_ = verts_[cur.x - 1];
					vert.tex_ = texs_[cur.y - 1];
					mesh->addVert(vert);
				} else {
					//inds_.push_back(combos_[cur]);
					mesh->addInd(combos_[cur]);
				}
			}
			read++;
		}
		fgetpos(file, &lastpos);
	}
	//~ printf("read %u faces\n", read); fflush(stdout);
	mesh->finalize();
	// we've read all the faces, so make the mesh
	return new ObjMesh(mesh);
}

Obj::ObjMesh* Obj::createPNMesh(FILE *file)
{
	currentcombo_ = 0;
	combos_.clear();
    //printf("creating pn mesh\n"); fflush(stdout);
	InterleavedMesh<PNvert, GLuint> *mesh = new InterleavedMesh<PNvert, GLuint>(GL_TRIANGLES);
	// 2 vertex attributes with 3 floats each
	mesh->addAttrib(0, AttributeInfoSpec<GLfloat>(3)).addAttrib(2, AttributeInfoSpec<GLfloat>(3));
	fpos_t lastpos;
	char buf[512] = "";
	fgetpos(file, &lastpos);
	unsigned int faces = 0;
	while (fscanf(file, "%s", buf) > 0) {
		if (strcmp(buf, "f") != 0) {
			// we hit something other than "f"
            //printf("hit something other than f, ending mesh\n"); fflush(stdout);
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
					//inds_.push_back(currentcombo_);
					mesh->addInd(currentcombo_);
					currentcombo_++;
					vert.pos_ = verts_[cur.x - 1];
					vert.norm_ = norms_[cur.y - 1];
					mesh->addVert(vert);
				} else {
					//inds_.push_back(combos_[cur]);
					mesh->addInd(combos_[cur]);
				}
			}
			faces++;
		}
		fgetpos(file, &lastpos);
	}
    //printf("read %u faces\n", faces); fflush(stdout);
	mesh->finalize();
	// we've read all the faces, so make the mesh
	return new ObjMesh(mesh);
}

Obj::ObjMesh* Obj::createPMesh(FILE *file)
{
	currentcombo_ = 0;
	combos_.clear();
	InterleavedMesh<fl3, GLuint> *mesh = new InterleavedMesh<fl3, GLuint>(GL_TRIANGLES);
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
					//inds_.push_back(currentcombo_);
					mesh->addInd(currentcombo_);
					currentcombo_++;
					vert = verts_[cur.x - 1];
					mesh->addVert(vert);
				} else {
					//inds_.push_back(combos_[cur]);
					mesh->addInd(combos_[cur]);
				}
			}
		}
		fgetpos(file, &lastpos);
	}
	mesh->finalize();
	// we've read all the faces, so make the mesh
	return new ObjMesh(mesh);
}

#ifndef OBJ_H
#define OBJ_H
#include <vector>
#include "utils.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <gl/gl.h>
#include <map>
#include "texture.h"
#include "mesh.h"
class Shader;
class MatrixStack;
/*
*	Class for loading obj mesh from a file
*	Used http://www.artifactgames.de/Code/Loader.zip as a starting point
*	As of now, only support triangle meshes
*
*	One OBJ mesh can contain multiple Meshes, since each sub mesh can use its own texture map, etc.
*	for the purposes of assigning vertex attributes, 0 is position, 1 is texcoord, 2 is normal
*/

class Obj
{
	public:
		Obj(const char *filename);
		virtual ~Obj();
	
		void draw(Shader &shader);
		void getBounds(fl3 &min, fl3 &max);
		
	private:
		// bounding box
		fl3 min_, max_;
		
		// struct for materials
		struct ObjMaterial {
			std::string name;
			float Ns; // specular coefficient
			float Ni; // index of refraction
			union {
				float d, Tr; // transparency (can have both notations)
			};
			fl3 Tf; // transmission filter (allows only certain colors through)
			
			unsigned int illum; // illumination model
			// 0 means constant illumination (color = Kd)
			// 1 means lambertian model (diffuse and ambient only)
			// 2 means lambert + blinn-phong (diffuse, specular, and ambient)
			// there's more at http://en.wikipedia.org/wiki/Wavefront_.obj_file
			// but these are the basics
			
			fl3 Ka; // ambient color
			fl3 Kd; // diffuse color
			fl3 Ks; // specular color
			fl3 Ke; // emissive color
			
			Texture *map_Ka; // ambient texture
			Texture *map_Kd; // diffuse texture
			Texture *map_Ks; // specular texture
		};
		
		// internal abstract class for handling different vertex formats
		struct ObjMesh {
			virtual void draw() = 0;
		};
		struct PTNMesh : ObjMesh {
			PTNMesh(Mesh<PTNvert, GLuint> *m) : mesh(m) {}
			virtual ~PTNMesh() { delete mesh; }
			void draw() { mesh->draw(); }
			Mesh<PTNvert, GLuint> *mesh;
		};
		struct PTMesh : ObjMesh {
			PTMesh(Mesh<PTvert, GLuint> *m) : mesh(m) {}
			virtual ~PTMesh() { delete mesh; }
			void draw() { mesh->draw(); }
			Mesh<PTvert, GLuint> *mesh;
		};
		struct PNMesh : ObjMesh {
			PNMesh(Mesh<PNvert, GLuint> *m) : mesh(m) {}
			virtual ~PNMesh() { delete mesh; }
			void draw() { mesh->draw(); }
			private:
			Mesh<PNvert, GLuint> *mesh;
		};
		struct PMesh : ObjMesh {
			PMesh(Mesh<fl3, GLuint> *m) : mesh(m) {}
			virtual ~PMesh() { delete mesh; }
			void draw() { mesh->draw(); };
			Mesh<fl3, GLuint> *mesh;
		};
		
		bool loadFile(const char *filename);
		bool loadMaterials(const char *filename);
		
		// functions to take the so far parsed data and turn it into a mesh
		ObjMesh* createPTNMesh(FILE *file);
		ObjMesh* createPTMesh(FILE *file);
		ObjMesh* createPNMesh(FILE *file);
		ObjMesh* createPMesh(FILE *file);
		
		// intermediate vectors for storing vertices, texcoords, normals
		std::vector<fl3> verts_;
		std::vector<fl3> texs_;
		std::vector<fl3> norms_;
		// faces/indices
		std::vector<GLuint> inds_;
	
		// temporary variables for parsing materials
		std::map<std::string, bool> mtlfiles_;
		// textures can be shared for multiple materials, so a map to keep track of them
		std::map<std::string, Texture *> textures_;
		// temporary map of which v/t/n combos have been assigned to which index
		std::map<int3, GLuint> combos_;
		GLuint currentcombo_;
		
		// map of materials by addressable name
		std::map<std::string, ObjMaterial *> materials_;
		
		// map for storing the final meshes and associated materials
		std::map<std::string, std::pair<ObjMesh *, ObjMaterial *>> meshes_;
};

#endif // OBJ_H
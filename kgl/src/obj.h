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
*
*	I originally planned to add functionality for making a separate vao/vbo with position-only data
*	for depth-prepass, but holding off on it for now
*/

class Obj
{
	public:
		Obj(const char *filename);
		virtual ~Obj();
	
        void draw(const Shader &shader) const;
		void getBounds(fl3 &min, fl3 &max);
		
	private:
		// bounding box
		fl3 min_, max_;
		
		// struct for materials
		struct ObjMaterial {
			virtual ~ObjMaterial() { glDeleteBuffers(1, &ubo); }
			struct ObjUniformBlock {
				// GLSL std140 layout compatible uniform block
				GLfloat Ns; // specular coefficient
				GLfloat Ni; // index of refraction
				union {
				GLfloat d, Tr; // transparency (can have both notations)
				};
				fl3 Tf; // transmission filter (allows only certain colors through)
				GLfloat padding0;
				
				GLuint illum; // illumination model
				// 0 means constant illumination (color = Kd)
				// 1 means lambertian model (diffuse and ambient only)
				// 2 means lambert + blinn-phong (diffuse, specular, and ambient)
				// there's more at http://en.wikipedia.org/wiki/Wavefront_.obj_file
				// but these are the basics
				
				fl3 Ka; // ambient color
				GLfloat padding1;
				fl3 Kd; // diffuse color
				GLfloat padding2;
				fl3 Ks; // specular color
				GLfloat padding3;
				fl3 Ke; // emissive color
				GLfloat padding4;
				
				//GLint sampler_Ka;
				//GLint sampler_Kd;
				//GLint sampler_Ks;
			} ublock;
			
			std::string name;
			
			Texture *map_Ka; // ambient texture
			Texture *map_Kd; // diffuse texture
			Texture *map_Ks; // specular texture
			
			GLuint ubo; // uniform buffer handle
		};
		
		// container for obj meshes to make them more controlled/static
		struct ObjMesh {
			ObjMesh(Mesh<GLuint> *m) : mesh(m) {};
			virtual ~ObjMesh() { delete mesh; }
			void draw() { mesh->draw(); }
			Mesh<GLuint> *mesh;
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
		// uniform buffers for these materials
		
		// map for storing the final meshes and associated materials
		std::map<std::string, std::pair<ObjMesh *, ObjMaterial *>> meshes_;
};

#endif // OBJ_H

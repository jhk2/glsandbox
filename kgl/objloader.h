#ifndef OBJLOADER_H
#define OBJLOADER_H
#include <vector>
#include "utils.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <gl/gl.h>
/*
*	Class for loading obj mesh from a file
*	Used http://www.artifactgames.de/Code/Loader.zip as a starting point
*	As of now, only support triangle meshes
*
*	One OBJ mesh can contain multiple Meshes, since each sub mesh can use its own texture map, etc.
*/

template<class V, typename I> class Mesh;

class ObjLoader
{
	public:
		ObjLoader(const char *filename);
		virtual ~ObjLoader();
		
		struct PTNvert {
			fl3 pos_;
			fl3 tex_;
			fl3 norm_;
		};
		struct PTvert {
			fl3 pos_;
			fl3 tex_;
		}
		struct PNvert {
			fl3 pos_;
			fl3 norm_;
		}
		
		
	private:		
		bool loadFile(const char *filename);
		bool loadMaterials(const char *filename);
		bool loadGeometry(const char *filename);
		
		// intermediate vectors for storing vertices, texcoords, normals
		std::vector<fl3> verts_;
		std::vector<fl3> texs_;
		std::vector<fl3> norms_;
		// faces/indices
		std::vector<GLuint> inds_;
};

#endif // OBJLOADER_H
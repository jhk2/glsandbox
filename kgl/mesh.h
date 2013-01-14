#ifndef MESH_H
#define MESH_H

#include "glextensionfuncs.h"
#include <map>
#include <vector>
#include <utility>

/*
*	learned about type erasure here
*	http://www.cplusplus.com/forum/articles/18756/
*/
class VertexBuffer
{
	VertexBuffer() {}
	virtual ~VertexBuffer() {}
	// getter for vertex data for use with glBufferData
	virtual const GLvoid* data() = 0;
	virtual unsigned int size() = 0;
};
// V is a custom vertex struct with all of the attributes defined in order
template<typename V> class VertexBufferSpec : VertexBuffer
{
	public:
		VertexBufferSpec(std::vector<V> *vertices) : VertexBuffer() { vertices_ = vertices; }
		virtual ~VertexBufferSpec() {}
		const GLvoid* data() { return (GLvoid*) vertices_->data(); }
		unsigned int size() { return vertices_->size(); }
	private:
		std::vector<V> *vertices_;
};

struct AttributeInfo
{
	AttributeInfo(const GLenum n, const unsigned int nc) : name(n), numComponents(nc) {}
	virtual ~AttributeInfo() {}
	GLenum name; // enum name associated with data type (e.g. GL_UNSIGNED_INT, GL_FLOAT)
	unsigned int numComponents; // number of components (>1 if the attribute is a vector)
	virtual size_t getSize() = 0; // size of the entire attribute in bytes, used to calculate offsets
};
// GL_TYPEDEF is the opengl typedef (e.g. GLuint, GLfloat)
template<typename GL_TYPEDEF> struct AttributeInfoSpec : AttributeInfo
{
	AttributeInfoSpec(const GLenum n, const unsigned int nc) : AttributeInfo(n, nc) {}
	virtual ~AttributeInfoSpec() {}
	size_t getSize() { return sizeof(GL_TYPEDEF) * numComponents; }
};

/*
*	Class for representing geometry data for a mesh (vertex array object, vertex buffers, etc.)
*	Should do all of the necessary vertex loading and setup so that we can easily draw
*	Will be inherited by various primitives and the like
*
*	Ideally, a mesh is made of up vertices which can have any number of vertex attributes
*	We require vertex attributes to be interleaved before being loaded, for now
*/
template<class V> // V is a custom vertex struct
class Mesh
{
	public:
		// attribInfo is a vector containing pairs of (attribute number, attribute info)
		// it should have the same order as the attributes are defined in the custom vertex struct V
			Mesh(std::vector< std::pair< unsigned int, AttributeInfo> > &attribInfo, 
				VertexBufferSpec<V> &vertices, std::vector<unsigned int> &indices);
		// TODO: make type of indices customizable, or force to GLuint
		virtual ~Mesh();
	
	protected:
		
		GLuint vao_, vbo_, ibo_;
		GLenum drawType_;
		GLuint idxCount_;
		
		// map of vertex attributes to offsets
		std::map<unsigned int, unsigned int> attribOffsets_;
};
#endif // MESH_H
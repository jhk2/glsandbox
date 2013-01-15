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
	public:
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
		Mesh(std::vector< std::pair< unsigned int, AttributeInfo*> > &attribInfo, 
			VertexBufferSpec<V> &vertices, std::vector<unsigned int> &indices)
		{
			printf("gen vertex arrays %p\n", glGenVertexArrays); fflush(stdout);
			glGenVertexArrays(1, &vao_);
			printf("bind vertex array %p\n", glBindVertexArray); fflush(stdout);
			glBindVertexArray(vao_);
			printf("done with vao\n"); fflush(stdout);
			
			// TODO: make this customizable
			drawType_ = GL_QUADS;
			idxCount_ = indices.size();
			printf("idxcount is %i\n", idxCount_); fflush(stdout);
			
			glGenBuffers(1, &vbo_);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_);
			printf("vbo generated\n"); fflush(stdout);
			glBufferData(GL_ARRAY_BUFFER, sizeof(V)*vertices.size(), vertices.data(), GL_STATIC_DRAW);
			printf("glbufferdata finished\n"); fflush(stdout);
			
			unsigned int offset = 0;
			printf("iterating through attribinfo size %i\n", attribInfo.size()); fflush(stdout);
			for(unsigned int i = 0; i < attribInfo.size(); i++) {
				printf("current index = $i\n", i); fflush(stdout);
				std::pair<unsigned int, AttributeInfo*> &cur = attribInfo[i];
				printf("enabling vertex attribute #%i\n", cur.first); fflush(stdout);
				glEnableVertexAttribArray(cur.first);
				// TODO: enable normalization?
				glVertexAttribPointer(cur.first, cur.second->numComponents, cur.second->name, GL_FALSE, sizeof(V), (char *)NULL + offset);
				printf("vertex attrib pointer with %i components and offset of %i\n", cur.second->numComponents, offset); fflush(stdout);
				offset += cur.second->getSize();
			}
			printf("finished setting up vertex attribs\n"); fflush(stdout);
			glGenBuffers(1, &ibo_);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
			// TODO: either force unsigned int or make index data type customizable
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*idxCount_, indices.data(), GL_STATIC_DRAW);
			printf("generated and filled ibo\n"); fflush(stdout);
			
			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
		// TODO: make type of indices customizable, or force to GLuint
		virtual ~Mesh() {};
	
		void draw()
		{
			glBindVertexArray(vao_);
			glDrawElements(drawType_, idxCount_, GL_UNSIGNED_INT, (char*) NULL + 0);
			glBindVertexArray(0);
		}
	
	protected:
		
		GLuint vao_, vbo_, ibo_;
		GLenum drawType_;
		GLuint idxCount_;
		
		// map of vertex attributes to offsets
		std::map<unsigned int, unsigned int> attribOffsets_;
};
#endif // MESH_H
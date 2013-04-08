#ifndef MESH_H
#define MESH_H

#include "glextensionfuncs.h"
#include <map>
#include <vector>
#include <utility>
#include <string>

/*
*	learned about type erasure here
*	http://www.cplusplus.com/forum/articles/18756/
*/

/*
*	Opengl type to openGL enum conversion using explicit template specialization
*	http://en.wikipedia.org/wiki/Template_(C%2B%2B)#Explicit_template_specialization
*	Idea from https://github.com/evanw/gl4/blob/master/gl4.h
*/
template <typename T> struct GLTypeToEnum {};
template<> struct GLTypeToEnum<GLfloat> { enum { value = GL_FLOAT }; };
template<> struct GLTypeToEnum<GLuint> { enum { value = GL_UNSIGNED_INT }; };
template<> struct GLTypeToEnum<GLushort> { enum { value = GL_UNSIGNED_SHORT }; };
template<> struct GLTypeToEnum<GLubyte> { enum { value = GL_UNSIGNED_BYTE }; };

// V is a custom vertex struct with all of the attributes defined in order
template<typename V> class VertexBufferSpec
{
	public:
		VertexBufferSpec() {}
		virtual ~VertexBufferSpec() {}
		const GLvoid* data() { return (GLvoid*) vertices_.data(); }
		const unsigned int size() { return vertices_.size(); }
		void addVerts(std::vector<V> &newverts) { vertices_.insert(vertices_.end(), newverts.begin(), newverts.end()); }
		void addVert(V &newvert) { vertices_.push_back(newvert); }
	private:
		std::vector<V> vertices_;
};

// type erasure thing for generic vertex attributes
struct AttributeInfoConcept
{
	AttributeInfoConcept(const unsigned int nc) : numComponents(nc) {}
	virtual ~AttributeInfoConcept() {}
	virtual GLenum name() = 0;
	unsigned int numComponents; // number of components (>1 if the attribute is a vector)
	virtual size_t getSize() = 0; // size of the entire attribute in bytes, used to calculate offsets
};

// GL_TYPEDEF is the opengl typedef (e.g. GLuint, GLfloat)
template<typename GL_TYPEDEF> struct AttributeInfoSpec : AttributeInfoConcept
{
	AttributeInfoSpec(const unsigned int nc) : AttributeInfoConcept(nc) {}
	virtual ~AttributeInfoSpec() {}
	GLenum name() { return GLTypeToEnum<GL_TYPEDEF>::value; }
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
template<class V, typename I> // V is a custom vertex struct, I is the type of indices (unsigned int, short, etc)
class Mesh
{
	public:
		// attribInfo is a vector containing pairs of (attribute number, attribute info)
		// it should have the same order as the attributes are defined in the custom vertex struct V
		Mesh(GLenum drawtype) : drawType_(drawtype), verts_(), attribs_(), inds_() {}

		virtual ~Mesh() 
		{
			for(int i = 0; i < attribs_.size(); i++) {
				delete attribs_[i].second;
			}
			glDeleteBuffers(1, &vbo_);
			glDeleteBuffers(1, &ibo_);
			glDeleteVertexArrays(1, &vao_);
		};
		
		template<typename T> Mesh& addAttrib(GLuint location, AttributeInfoSpec<T> &spec)
		{
			attribs_.push_back(std::pair<GLuint, AttributeInfoConcept*>(location, new AttributeInfoSpec<T>(spec.numComponents)));
			return *this;
		}
		
		Mesh& addVert(V &newvert)
		{
			verts_.addVert(newvert);
			return *this;
		}
		
		Mesh& addVerts(std::vector<V> &newverts)
		{
			verts_.addVerts(newverts);
			return *this;
		}
		
		Mesh& addInd(I &newind)
		{
			inds_.push_back(newind);
			return *this;
		}
		
		Mesh& addInds(std::vector<I> &newinds)
		{
			inds_.insert(inds_.end(), newinds.begin(), newinds.end());
			return *this;
		}
		
		// after setting up all of the vertex attributes and populating vertex and index vectors,
		// use this method to push all of the data to GL
		void finalize()
		{
			//~ printf("gen vertex arrays %p\n", glGenVertexArrays); fflush(stdout);
			glGenVertexArrays(1, &vao_);
			//~ printf("bind vertex array %p\n", glBindVertexArray); fflush(stdout);
			glBindVertexArray(vao_);
			//~ printf("done with vao\n"); fflush(stdout);
			
			// TODO: make this customizable
			// drawType_ = GL_QUADS; it should be now in constructor
			idxCount_ = inds_.size();
			//~ printf("idxcount is %i\n", idxCount_); fflush(stdout);
			
			glGenBuffers(1, &vbo_);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_);
			//~ printf("vbo generated\n"); fflush(stdout);
			glBufferData(GL_ARRAY_BUFFER, sizeof(V)*verts_.size(), verts_.data(), GL_STATIC_DRAW);
			//~ printf("glbufferdata finished\n"); fflush(stdout);
			
			unsigned int offset = 0;
			//~ printf("iterating through attribinfo size %i\n", attribs_.size()); fflush(stdout);
			for(unsigned int i = 0; i < attribs_.size(); i++) {
				//~ printf("current index = %i\n", i); fflush(stdout);
				std::pair<unsigned int, AttributeInfoConcept*> &cur = attribs_[i];
				//~ printf("enabling vertex attribute #%i\n", cur.first); fflush(stdout);
				glEnableVertexAttribArray(cur.first);
				// TODO: enable normalization?
				glVertexAttribPointer(cur.first, cur.second->numComponents, cur.second->name(), GL_FALSE, sizeof(V), (char *)NULL + offset);
				//~ printf("vertex attrib pointer with %i components and offset of %i\n", cur.second->numComponents, offset); fflush(stdout);
				offset += cur.second->getSize();
			}
			//~ printf("finished setting up vertex attribs\n"); fflush(stdout);
			glGenBuffers(1, &ibo_);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
			// TODO: either force unsigned int or make index data type customizable
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(I)*idxCount_, inds_.data(), GL_STATIC_DRAW);
			//~ printf("generated and filled ibo\n"); fflush(stdout);
			
			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		};
	
		void draw()
		{
			glBindVertexArray(vao_);
			glDrawElements(drawType_, idxCount_, GLTypeToEnum<I>::value, (char*) NULL + 0);
			glBindVertexArray(0);
		}
	
	protected:
		
		GLuint vao_, vbo_, ibo_;
		GLenum drawType_;
		GLuint idxCount_;
		
		std::vector<std::pair<GLuint, AttributeInfoConcept*>> attribs_;
		VertexBufferSpec<V> verts_;
		std::vector<I> inds_;
};
#endif // MESH_H
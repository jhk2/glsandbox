#include "mesh.h"
template<class V>
Mesh<V>::Mesh(std::vector< std::pair< unsigned int, AttributeInfo > > &attribInfo, 
	VertexBufferSpec<V> &vertices, std::vector<unsigned int> &indices)
{
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);
	
	// TODO: make this customizable
	drawType_ = GL_TRIANGLES;
	idxCount_ = indices.size();
	
	glGenBuffers(1, &vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(V)*vertices.size(), vertices.data(), GL_STATIC_DRAW);
	
	unsigned int offset = 0;
	for(unsigned int i = 0; i < attribInfo.size(); i++) {
		std::pair<unsigned int, AttributeInfo> &cur = attribInfo[i];
		glEnableVertexAttribArray(cur.first);
		// TODO: enable normalization?
		glVertexAttribPointer(cur.first, cur.second.name, GL_FALSE, cur.second.getSize(), (char *)NULL + offset);
		offset += second.getSize();
	}
	
	glGenBuffers(1, &ibo_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
	// TODO: either force unsigned int or make index data type customizable
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*idxCount_, indices.data(), GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

template<class V>
Mesh<V>::~Mesh()
{
	
}
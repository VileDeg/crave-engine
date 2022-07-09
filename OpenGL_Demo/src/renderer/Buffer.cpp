#include "pch.h"

#include "Buffer.h"
#include "glad/glad.h"


/////////////////////////////////////////////////////////
//VBO////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

VBO::VBO(const void* data, std::size_t size) : count(-1)
{
	glGenBuffers(1, &id);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

VBO::VBO(const void* data, const std::size_t size, const int vertexCount)
{
	glGenBuffers(1, &id);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	count = vertexCount;
}

VBO::~VBO()
{
	glDeleteBuffers(1, &id);
}


void VBO::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, id);
}

void VBO::Unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/////////////////////////////////////////////////////////
//Layout////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

const unsigned VertexLayout::VertexAttribute::GetTypeSize() const
{
	switch (type)
	{
	case GL_FLOAT:         return sizeof(GLfloat);
	case GL_UNSIGNED_INT:  return sizeof(GLuint);
	case GL_UNSIGNED_BYTE: return sizeof(GLbyte);
	}

	return 0;
}

VertexLayout::VertexLayout(const std::initializer_list<VertexAttribute>& list)
: stride(0), attribs(list)
{
	for (auto& attrib : attribs)
	{
		stride += attrib.count * sizeof(attrib.GetTypeSize());
	}
}

void VertexLayout::Enable() const
{
	unsigned offset = 0;
	for (unsigned i = 0; i < attribs.size(); ++i)
	{
		const auto& at = attribs[i];
		glVertexAttribPointer(i, at.count, at.type, at.normalized, stride, (void*)offset);
		glEnableVertexAttribArray(i);
		offset += at.count * at.GetTypeSize();
	}
}


/////////////////////////////////////////////////////////
//VAO////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

VAO::VAO()
{
	glGenVertexArrays(1, &id);
}

VAO::~VAO()
{
	glDeleteVertexArrays(1, &id);
}

void VAO::AddBuffer(const VBO& vbo)
{
	Bind();
	vbo.Bind();
	vbo.GetLayout().Enable();
	count = vbo.Count();
}

void VAO::Bind() const
{
	glBindVertexArray(id);
}

void VAO::Unbind() const
{
	glBindVertexArray(0);
}




/////////////////////////////////////////////////////////
//EBO////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

EBO::EBO(const unsigned int* data, unsigned int count) : count(count)
{
    glGenBuffers(1, &id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
}

EBO::~EBO()
{
    glDeleteBuffers(1, &id);
}


void EBO::Bind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}

void EBO::Unbind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/////////////////////////////////////////////////////////
//UBO////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

UBO::UBO(const char* name, const void* data, const std::size_t size)
	: m_Name(name)
{
	glGenBuffers(1, &m_Id);
	glBindBuffer(GL_UNIFORM_BUFFER, m_Id);
	glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);
}

UBO::~UBO()
{
	glDeleteBuffers(1, &m_Id);
}


void UBO::Upload(const void* data, const std::size_t size, const unsigned offset)
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_Id);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

}

void UBO::Bind(unsigned bindingPoint)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_Id);
}

void UBO::Unbind()
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
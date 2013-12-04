#include "elematerendervertexbuffer.h"

#include <iostream>
#include <cassert>

#include <osg/BufferObject>
#include <osg/Camera>

namespace physx {
namespace apex {


ElemateRenderVertexBuffer::ElemateRenderVertexBuffer(osg::GraphicsContext & context)
: m_osgGraphicsContext(&context)
, m_osgVBO(osg::ref_ptr<osg::VertexBufferObject>(new osg::VertexBufferObject))
{
    std::cout << "creating vertex buffer" << std::endl;
}

ElemateRenderVertexBuffer::~ElemateRenderVertexBuffer()
{
    std::cout << "deleting vertex buffer" << std::endl;
}

unsigned int ElemateRenderVertexBuffer::contextID() const
{
    assert(m_osgGraphicsContext.valid());
    return m_osgGraphicsContext->getState()->getContextID();
}

bool ElemateRenderVertexBuffer::getInteropResourceHandle(CUgraphicsResource& handle)
{
    std::cout << "get vertex buffer handle" << std::endl;
    assert(m_osgVBO.valid());

    const GLuint id = m_osgVBO->getGLBufferObject(contextID())->getGLObjectID();

    // CUgraphicsResource wtf?
    // use this for now, but no idea if this works
    return NxUserRenderVertexBuffer::getInteropResourceHandle(handle);

    return false;
}

void ElemateRenderVertexBuffer::writeBuffer(const physx::NxApexRenderVertexBufferData& data, physx::PxU32 firstVertex, physx::PxU32 numVertices)
{
    std::cout << "write vertex buffer" << std::endl;
    PxU32 numData = data.getNumCustomSemantics();
}


}
}
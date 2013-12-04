#pragma once

#include <NxUserRenderVertexBuffer.h>

#include <osg/GraphicsContext>

namespace osg {
    class VertexBufferObject;
}

namespace physx {
namespace apex {

class ElemateRenderVertexBuffer : public NxUserRenderVertexBuffer {
public:
    ElemateRenderVertexBuffer() = delete;
    ElemateRenderVertexBuffer(osg::GraphicsContext & context);
    virtual ~ElemateRenderVertexBuffer() override;

    virtual bool getInteropResourceHandle(CUgraphicsResource& handle) override;

    virtual void writeBuffer(const physx::NxApexRenderVertexBufferData& data, physx::PxU32 firstVertex, physx::PxU32 numVertices) override;

private:
    unsigned int contextID() const;
    osg::ref_ptr<osg::GraphicsContext> m_osgGraphicsContext;
    osg::ref_ptr<osg::VertexBufferObject> m_osgVBO;
};


}
}
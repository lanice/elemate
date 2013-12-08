#pragma once

#include <NxUserRenderResourceManager.h>

#include <osg/GraphicsContext>

namespace physx {
namespace apex {

class ElemateRenderResourceManager : public physx::apex::NxUserRenderResourceManager
{
public:
    ElemateRenderResourceManager() = delete;
    ElemateRenderResourceManager(osg::GraphicsContext & context);
    virtual ~ElemateRenderResourceManager() override;

    void setOsgGraphicsContext(osg::GraphicsContext & context);

    virtual NxUserRenderVertexBuffer* createVertexBuffer(const NxUserRenderVertexBufferDesc& desc) override;

    // nope
    virtual void                        releaseVertexBuffer(NxUserRenderVertexBuffer& buffer) override;
    virtual NxUserRenderIndexBuffer*    createIndexBuffer(const NxUserRenderIndexBufferDesc& desc) override;
    virtual void                        releaseIndexBuffer(NxUserRenderIndexBuffer& buffer) override;
    virtual NxUserRenderBoneBuffer*     createBoneBuffer(const NxUserRenderBoneBufferDesc& desc) override;
    virtual void                        releaseBoneBuffer(NxUserRenderBoneBuffer& buffer) override;
    virtual NxUserRenderInstanceBuffer* createInstanceBuffer(const NxUserRenderInstanceBufferDesc& desc) override;
    virtual void                        releaseInstanceBuffer(NxUserRenderInstanceBuffer& buffer) override;
    virtual NxUserRenderSpriteBuffer*   createSpriteBuffer(const NxUserRenderSpriteBufferDesc& desc) override;
    virtual void                        releaseSpriteBuffer(NxUserRenderSpriteBuffer& buffer) override;
    virtual NxUserRenderResource*       createResource(const NxUserRenderResourceDesc& desc) override;
    virtual void                        releaseResource(NxUserRenderResource& resource) override;
    virtual physx::PxU32                getMaxBonesForMaterial(void* material) override;

private:
    osg::ref_ptr<osg::GraphicsContext> m_osgGraphicsContext;
};

}
}
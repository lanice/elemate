#include "elematerenderresourcemanager.h"

#include <cassert>

namespace physx {
namespace apex {

ElemateRenderResourceManager::ElemateRenderResourceManager(osg::GraphicsContext & context)
: m_osgGraphicsContext(&context)
{
}

ElemateRenderResourceManager::~ElemateRenderResourceManager()
{
}

void ElemateRenderResourceManager::setOsgGraphicsContext(osg::GraphicsContext & context)
{
    m_osgGraphicsContext = &context;
}

NxUserRenderVertexBuffer* ElemateRenderResourceManager::createVertexBuffer(const NxUserRenderVertexBufferDesc& desc)
{
    assert(false); exit(42);
    return nullptr;
}

void ElemateRenderResourceManager::releaseVertexBuffer(NxUserRenderVertexBuffer& buffer)
{
    assert(false); exit(42);
}

NxUserRenderIndexBuffer* ElemateRenderResourceManager::createIndexBuffer(const NxUserRenderIndexBufferDesc& desc)
{
    assert(false); exit(42);
    return nullptr;
}

void ElemateRenderResourceManager::releaseIndexBuffer(NxUserRenderIndexBuffer& buffer)
{
    assert(false); exit(42);
}

NxUserRenderBoneBuffer* ElemateRenderResourceManager::createBoneBuffer(const NxUserRenderBoneBufferDesc& desc)
{
    assert(false); exit(42);
    return  nullptr;
}

void ElemateRenderResourceManager::releaseBoneBuffer(NxUserRenderBoneBuffer& buffer)
{
    assert(false); exit(42);
}

NxUserRenderInstanceBuffer* ElemateRenderResourceManager::createInstanceBuffer(const NxUserRenderInstanceBufferDesc& desc)
{
    assert(false); exit(42);
    return nullptr;
}

void ElemateRenderResourceManager::releaseInstanceBuffer(NxUserRenderInstanceBuffer& buffer)
{
    assert(false); exit(42);
}

NxUserRenderSpriteBuffer*   ElemateRenderResourceManager::createSpriteBuffer(const NxUserRenderSpriteBufferDesc& desc)
{
    assert(false); exit(42);
    return nullptr;
}

void ElemateRenderResourceManager::releaseSpriteBuffer(NxUserRenderSpriteBuffer& buffer)
{
    assert(false); exit(42);
}

NxUserRenderResource* ElemateRenderResourceManager::createResource(const NxUserRenderResourceDesc& desc)
{
    assert(false); exit(42);
    return nullptr;
}

void ElemateRenderResourceManager::releaseResource(NxUserRenderResource& resource)
{
    assert(false); exit(42);
}

physx::PxU32 ElemateRenderResourceManager::getMaxBonesForMaterial(void* material)

{
    assert(false); exit(42);
    return 0;
}

physx::PxU32 ElemateRenderResourceManager::getSpriteTextureData(physx::PxU32 spriteCount, physx::PxU32 spriteSemanticsBitmap, NxUserRenderSpriteTextureDesc* textureDescArray)
{
    assert(false); exit(42);
    PX_FORCE_PARAMETER_REFERENCE(spriteCount);
    PX_FORCE_PARAMETER_REFERENCE(spriteSemanticsBitmap);
    PX_FORCE_PARAMETER_REFERENCE(textureDescArray);
    return 0;
}


}
}
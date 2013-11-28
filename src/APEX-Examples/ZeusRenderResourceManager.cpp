#include "ZeusRenderResourceManager.h"

ZeusRenderResourceManager::ZeusRenderResourceManager(ID3D11Device* dev, ID3D11DeviceContext* devcon) :
    mDevice(dev), mDevcon(devcon)
{
    
}

ZeusRenderResourceManager::~ZeusRenderResourceManager()
{

}

physx::apex::NxUserRenderVertexBuffer* ZeusRenderResourceManager::createVertexBuffer(const physx::apex::NxUserRenderVertexBufferDesc& desc)
{
    ZeusVertexBuffer* vbuff = new ZeusVertexBuffer(desc, mDevice, mDevcon);
	m_numVertexBuffers++;
	return (NxUserRenderVertexBuffer*)vbuff;
}

void ZeusRenderResourceManager::releaseVertexBuffer(physx::apex::NxUserRenderVertexBuffer& buffer)
{
	PX_ASSERT(m_numVertexBuffers > 0);
	m_numVertexBuffers--;
	delete &buffer;
}


physx::apex::NxUserRenderIndexBuffer* ZeusRenderResourceManager::createIndexBuffer(const physx::apex::NxUserRenderIndexBufferDesc& desc)
{
    ZeusIndexBuffer* indbuff = new ZeusIndexBuffer(desc, mDevice, mDevcon);
	m_numIndexBuffers++;
    return indbuff;
}

void ZeusRenderResourceManager::releaseIndexBuffer(physx::apex::NxUserRenderIndexBuffer& buffer)
{
	PX_ASSERT(m_numIndexBuffers > 0);
	m_numIndexBuffers--;
	delete &buffer;
}


physx::apex::NxUserRenderSurfaceBuffer* ZeusRenderResourceManager::createSurfaceBuffer(const physx::apex::NxUserRenderSurfaceBufferDesc& desc)
{
    ZeusSurfaceBuffer* buffer = 0;
	m_numSurfaceBuffers++;
	return (NxUserRenderSurfaceBuffer*)buffer;
}

void ZeusRenderResourceManager::releaseSurfaceBuffer(physx::apex::NxUserRenderSurfaceBuffer& buffer)
{
	PX_ASSERT(m_numSurfaceBuffers > 0);
	m_numSurfaceBuffers--;
	delete &buffer;
}


physx::apex::NxUserRenderBoneBuffer* ZeusRenderResourceManager::createBoneBuffer(const physx::apex::NxUserRenderBoneBufferDesc& desc)
{
    ZeusBoneBuffer* buffer = 0;
	m_numBoneBuffers++;
	return (NxUserRenderBoneBuffer*)buffer;
}

void ZeusRenderResourceManager::releaseBoneBuffer(physx::apex::NxUserRenderBoneBuffer& buffer)
{
	PX_ASSERT(m_numBoneBuffers > 0);
	m_numBoneBuffers--;
	delete &buffer;
}


physx::apex::NxUserRenderInstanceBuffer* ZeusRenderResourceManager::createInstanceBuffer(const physx::apex::NxUserRenderInstanceBufferDesc& desc)
{
    ZeusInstanceBuffer* buffer = 0;
	m_numBoneBuffers++;
	return (NxUserRenderInstanceBuffer*)buffer;
}

void ZeusRenderResourceManager::releaseInstanceBuffer(physx::apex::NxUserRenderInstanceBuffer& buffer)
{
	PX_ASSERT(m_numInstanceBuffers > 0);
	m_numInstanceBuffers--;
	delete &buffer;
}


physx::apex::NxUserRenderSpriteBuffer* ZeusRenderResourceManager::createSpriteBuffer(const physx::apex::NxUserRenderSpriteBufferDesc& desc)
{
    ZeusSpriteBuffer* buffer = new ZeusSpriteBuffer(desc, mDevice, mDevcon);
	m_numSpriteBuffers++;
	return (NxUserRenderSpriteBuffer*)buffer;
}

void ZeusRenderResourceManager::releaseSpriteBuffer(physx::apex::NxUserRenderSpriteBuffer& buffer)
{
	PX_ASSERT(m_numVertexBuffers > 0);
	m_numSpriteBuffers--;
	delete &buffer;
}


physx::apex::NxUserRenderResource* ZeusRenderResourceManager::createResource(const physx::apex::NxUserRenderResourceDesc& desc)
{
   	ZeusRenderResource* resource =  new ZeusRenderResource(desc);
	m_numResources++;
	
	return (NxUserRenderResource*)resource;
}

void ZeusRenderResourceManager::releaseResource(physx::apex::NxUserRenderResource& resource)
{
	PX_ASSERT(m_numResources > 0);
	m_numResources--;
	delete &resource;
}


physx::PxU32 ZeusRenderResourceManager::getMaxBonesForMaterial(void* material)
{
	return 0;
}

// change the material of a render resource
void setMaterial(physx::apex::NxUserRenderResource& resource, void* material)
{
	//static_cast<ZeusRenderResource&>(resource).setMaterial(material);
}


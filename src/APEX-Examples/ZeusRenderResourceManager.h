#ifndef RENDER_RESOURCE_MANAGER
#define RENDER_RESOURCE_MANAGER
#include "apex.h"
#include "ZeusRenderResources.h"
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>

class ZeusRenderResourceManager : public physx::apex::NxUserRenderResourceManager
{
public:
    ZeusRenderResourceManager(ID3D11Device* dev, ID3D11DeviceContext* devcon);
	virtual								~ZeusRenderResourceManager(void);

public:
	virtual physx::apex::NxUserRenderVertexBuffer*		createVertexBuffer(const physx::apex::NxUserRenderVertexBufferDesc& desc);
	virtual void										releaseVertexBuffer(physx::apex::NxUserRenderVertexBuffer& buffer);

	virtual physx::apex::NxUserRenderIndexBuffer*		createIndexBuffer(const physx::apex::NxUserRenderIndexBufferDesc& desc);
	virtual void										releaseIndexBuffer(physx::apex::NxUserRenderIndexBuffer& buffer);

	virtual physx::apex::NxUserRenderSurfaceBuffer*		createSurfaceBuffer(const physx::apex::NxUserRenderSurfaceBufferDesc& desc);
	virtual void										releaseSurfaceBuffer(physx::apex::NxUserRenderSurfaceBuffer& buffer);

	virtual physx::apex::NxUserRenderBoneBuffer*		createBoneBuffer(const physx::apex::NxUserRenderBoneBufferDesc& desc);
	virtual void										releaseBoneBuffer(physx::apex::NxUserRenderBoneBuffer& buffer);

	virtual physx::apex::NxUserRenderInstanceBuffer*	createInstanceBuffer(const physx::apex::NxUserRenderInstanceBufferDesc& desc);
	virtual void										releaseInstanceBuffer(physx::apex::NxUserRenderInstanceBuffer& buffer);

	virtual physx::apex::NxUserRenderSpriteBuffer*		createSpriteBuffer(const physx::apex::NxUserRenderSpriteBufferDesc& desc);
	virtual void										releaseSpriteBuffer(physx::apex::NxUserRenderSpriteBuffer& buffer);

	virtual physx::apex::NxUserRenderResource*			createResource(const physx::apex::NxUserRenderResourceDesc& desc);
	virtual void										releaseResource(physx::apex::NxUserRenderResource& resource);

	virtual physx::PxU32								getMaxBonesForMaterial(void* material);

	// change the material of a render resource
	void												setMaterial(physx::apex::NxUserRenderResource& resource, void* material);
protected:
	physx::PxU32				m_numVertexBuffers;
	physx::PxU32				m_numIndexBuffers;
	physx::PxU32				m_numSurfaceBuffers;
	physx::PxU32				m_numBoneBuffers;
	physx::PxU32				m_numInstanceBuffers;
	physx::PxU32				m_numSpriteBuffers;
	physx::PxU32				m_numResources;
    ID3D11Device*               mDevice;
    ID3D11DeviceContext*        mDevcon;
};

#endif
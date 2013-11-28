#pragma once

#include "NxResourceCallback.h"
#include "NxUserRenderResourceManager.h"

namespace physx{
    namespace apex{
        class NxUserRenderVertexBuffer;
        class NxUserRenderIndexBuffer;
        class NxUserRenderBoneBuffer;
        class NxUserRenderInstanceBuffer;
        class NxUserRenderSpriteBuffer;
    }
}


class NullRenderResourceManager : public physx::apex::NxUserRenderResourceManager
{
public:
	/**
		The create methods in this class will only be called from the context of an NxApexRenderable::updateRenderResources()
		call, but the release methods can be triggered by any APEX API call that deletes an NxApexActor.  It is up to
		the end-user to make the release methods thread safe.
	*/

    virtual physx::apex::NxUserRenderVertexBuffer*   createVertexBuffer(const physx::apex::NxUserRenderVertexBufferDesc& desc){
        return NULL;
    }
	/** \brief Release vertex buffer */
    virtual void                        releaseVertexBuffer(physx::apex::NxUserRenderVertexBuffer& buffer){
    }

	/** \brief Create index buffer */
    virtual physx::apex::NxUserRenderIndexBuffer*    createIndexBuffer(const physx::apex::NxUserRenderIndexBufferDesc& desc){
        return NULL;
    }
	/** \brief Release index buffer */
    virtual void                        releaseIndexBuffer(physx::apex::NxUserRenderIndexBuffer& buffer){
    }

	/** \brief Create bone buffer */
    virtual physx::apex::NxUserRenderBoneBuffer*     createBoneBuffer(const physx::apex::NxUserRenderBoneBufferDesc& desc){
        return NULL;
    }
	/** \brief Release bone buffer */
    virtual void                        releaseBoneBuffer(physx::apex::NxUserRenderBoneBuffer& buffer) {
    }

	/** \brief Create instance buffer */
    virtual physx::apex::NxUserRenderInstanceBuffer* createInstanceBuffer(const physx::apex::NxUserRenderInstanceBufferDesc& desc){
        return NULL;
    }
	/** \brief Release instance buffer */
    virtual void                        releaseInstanceBuffer(physx::apex::NxUserRenderInstanceBuffer& buffer){
    }

	/** \brief Create sprite buffer */
    virtual physx::apex::NxUserRenderSpriteBuffer*   createSpriteBuffer(const physx::apex::NxUserRenderSpriteBufferDesc& desc){
        return NULL;
    }
	/** \brief Release sprite buffer */
    virtual void                        releaseSpriteBuffer(physx::apex::NxUserRenderSpriteBuffer& buffer){
    }
    
	/** \brief Create resource */
    virtual physx::apex::NxUserRenderResource*       createResource(const physx::apex::NxUserRenderResourceDesc& desc){
        return NULL;
    }

	/**
	releaseResource() should not release any of the included buffer pointers.  Those free methods will be
	called separately by the APEX SDK before (or sometimes after) releasing the NxUserRenderResource.
	*/
    virtual void                        releaseResource(physx::apex::NxUserRenderResource& resource){
    }

	/**
	Get the maximum number of bones supported by a given material. Return 0 for infinite.
	For optimal rendering, do not limit the bone count (return 0 from this function).
	*/
    virtual physx::PxU32                       getMaxBonesForMaterial(void* material){
        return 0;
    }
};

class NullResourceCallback : public physx::apex::NxResourceCallback
{
public:
    NullResourceCallback(){};
    virtual ~NullResourceCallback(){};
    /**
    \brief Request a resource from the user

    Will be called by the ApexSDK if a named resource is required but has not yet been provided.
    The resource pointer is returned directly, NxResourceProvider::setResource() should not be called.
    This function will be called at most once per named resource, unless an intermediate call to
    releaseResource() has been made.
    
    \note If this call results in the application calling NxApexSDK::createAsset, the name given 
          to the asset must match the input name parameter in this method.
    */
    virtual void* requestResource(const char* nameSpace, const char* name){
        nameSpace; name; return NULL;
    }

    /**
    \brief Request the user to release a resource

    Will be called by the ApexSDK when all internal references to a named resource have been released.
    If this named resource is required again in the future, a new call to requestResource() will be made.
    */
    virtual void  releaseResource(const char* nameSpace, const char* name, void* resource){
        nameSpace; name;  resource;
    }

};
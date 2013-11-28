#ifndef ZEUS_RESOURCE_CALLBACK
#define ZEUS_RESOURCE_CALLBACK
#include "apex.h"

class ZeusResourceCallback : public physx::apex::NxResourceCallback
{
public:
    ZeusResourceCallback();
    virtual ~ZeusResourceCallback();
    /**
    \brief Request a resource from the user

    Will be called by the ApexSDK if a named resource is required but has not yet been provided.
    The resource pointer is returned directly, NxResourceProvider::setResource() should not be called.
    This function will be called at most once per named resource, unless an intermediate call to
    releaseResource() has been made.
    
    \note If this call results in the application calling NxApexSDK::createAsset, the name given 
          to the asset must match the input name parameter in this method.
    */
    virtual void* requestResource(const char* nameSpace, const char* name);

    /**
    \brief Request the user to release a resource

    Will be called by the ApexSDK when all internal references to a named resource have been released.
    If this named resource is required again in the future, a new call to requestResource() will be made.
    */
    virtual void  releaseResource(const char* nameSpace, const char* name, void* resource);
private:

};
#endif
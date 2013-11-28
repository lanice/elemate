#include "ZeusResourceCallback.h"

ZeusResourceCallback::ZeusResourceCallback()
{

}
    

ZeusResourceCallback::~ZeusResourceCallback()
{

}
    /**
    \brief Request a resource from the user

    Will be called by the ApexSDK if a named resource is required but has not yet been provided.
    The resource pointer is returned directly, NxResourceProvider::setResource() should not be called.
    This function will be called at most once per named resource, unless an intermediate call to
    releaseResource() has been made.
    
    \note If this call results in the application calling NxApexSDK::createAsset, the name given 
          to the asset must match the input name parameter in this method.
    */

void* ZeusResourceCallback::requestResource(const char* nameSpace, const char* name)
{
    void* resource = 0;

    PX_ASSERT(nameSpace && *nameSpace);
    PX_ASSERT(name && *name);

    // Right now only do .apb files
    //char fullname[512] = {0};
    //physx::string::strcat_s(fullname, sizeof(fullname), name);
    //physx::string::strcat_s(fullname, sizeof(fullname), ".apb");
    std::string filename = name + std::string(".apb");

    physx::PxFileBuf* stream = NxGetApexSDK()->createStream( filename.c_str(), physx::PxFileBuf::OPEN_READ_ONLY );
    
    if(stream)
    {
        bool success = stream->getOpenMode() == physx::PxFileBuf::OPEN_READ_ONLY;
        if(!success)
            return NULL;
        NxParameterized::Serializer::SerializeType serType = NxGetApexSDK()->getSerializeType(*stream);
        PX_ASSERT( serType != NxParameterized::Serializer::SerializeType::NST_LAST );

        NxParameterized::Serializer * ser = NxGetApexSDK()->createSerializer(serType);

        NxApexAsset *asset;
        NxParameterized::Serializer::DeserializedData data;

        ser->deserialize(*stream, data); // assume there is one asset in the stream for this case
        NxParameterized::Interface *params = data[0];
        asset = NxGetApexSDK()->createAsset( params, name );



        PX_ASSERT(asset);
        if (asset)
        {
            bool rightType = strcmp(nameSpace, asset->getObjTypeName()) == 0;
            PX_ASSERT(rightType);
            if (rightType)
            {
                resource = asset;
            }
            else
            {
                NxGetApexSDK()->releaseAsset(*asset);
                asset = 0;
            }
        }
    }

    return resource;
}

    /**
    \brief Request the user to release a resource

    Will be called by the ApexSDK when all internal references to a named resource have been released.
    If this named resource is required again in the future, a new call to requestResource() will be made.
    */
void  ZeusResourceCallback::releaseResource(const char* nameSpace, const char* name, void* resource)
{
    PX_ASSERT(resource);
    if (resource)
    {
        physx::apex::NxApexAsset* asset = (physx::apex::NxApexAsset*)resource;
        NxGetApexSDK()->releaseAsset(*asset);
    }
}
#include "elemateresourcecallback.h"

// mostly from MinimalTurbulence.cpp

#include <iostream>

#include <fieldsampler/public/NxModuleFieldSampler.h>
#include <emitter/public/NxApexEmitterAsset.h>

const std::string dataPath("data/");

namespace physx {
namespace apex {

ElemateResourceCallback::ElemateResourceCallback()
: m_apexSDK(nullptr)
{
    m_group_masks.emplace("EXAMPLE_FS_GRP_MASK", physx::apex::NxGroupsMask64(1, 0));
    m_group_masks.emplace("MESH_PARTICLE_FS_GRP_MASK", physx::apex::NxGroupsMask64(3, 0));

    PxFilterData g;
    g.word1 = g.word2 = g.word3 = 0;
    g.word0 = 1;
    m_simulation_filter_data.emplace("MESH_COL_GRP_MASK", g);
    g.word0 = 1;
    m_simulation_filter_data.emplace("SPRITE_COL_GRP_MASK",g);
    g.word0 = 3;
    m_simulation_filter_data.emplace("ALL_COL_GRP_MASK",g);
}

void ElemateResourceCallback::setApexSDK(NxApexSDK * sdk)
{
    m_apexSDK = sdk;
}

void * ElemateResourceCallback::requestResource(const char* nameSpace, const char* name)
{
    std::cout << "Requested ressource: " << nameSpace << "::" << name << std::endl;

    // if (std::string(nameSpace) == "NSCollisionGroup128")
    //     return createCollisionGroup128(name);

    return loadSingleResourceRaw(name);
}

void * ElemateResourceCallback::loadSingleResourceRaw(const std::string & name) const
{
    NxParameterized::Traits* traits = m_apexSDK->getParameterizedTraits();
    NxParameterized::Serializer* serializer = m_apexSDK->createSerializer(NxParameterized::Serializer::NST_BINARY, traits);

    std::string filename(dataPath + name + ".apb");


    PxFileBuf* fileStream = m_apexSDK->createStream(filename.c_str(), PxFileBuf::OPEN_READ_ONLY);
    if (!fileStream->isOpen())
    {
        std::cerr << "Error: requestResources failed to open " << filename << std::endl;
        return nullptr;
    }

    NxParameterized::Serializer::DeserializedData deserializedData;
    serializer->deserialize(*fileStream, deserializedData);
    if (1 != deserializedData.size())
    {
        std::cerr << "Error: requestResources found " << deserializedData.size() << " objects in " << filename << std::endl;
        return nullptr;
    }

    NxApexAsset* asset = m_apexSDK->createAsset(deserializedData[0], name.c_str());
    if (!asset)
    {
        std::cerr << "Error: requestResources failed to create asset from " << filename << std::endl;
        return nullptr;
    }

    return asset;
}

void ElemateResourceCallback::releaseResource(const char* nameSpace, const char* name, void* resource)
{
    std::cerr << "Requested release of unknown ressource: " << nameSpace << "::" << name << " (at: " << resource << ")" << std::endl;
}

}
}
#include "elemateresourcecallback.h"

// mostly from MinimalTurbulence.cpp

#include <iostream>

#include <NxApexSDK.h>
#include <emitter/public/NxApexEmitterAsset.h>

const std::string dataPath("data/");

namespace physx {
namespace apex {

ElemateResourceCallback::ElemateResourceCallback()
: m_apexSDK(nullptr)
{
}

void ElemateResourceCallback::setApexSDK(NxApexSDK * sdk)
{
    m_apexSDK = sdk;
}

void * ElemateResourceCallback::requestResource(const char* nameSpace, const char* name)
{
    std::cout << "Requested ressource: " << nameSpace << "::" << name << std::endl;

    if (std::string(nameSpace) == "NSCollisionGroup128")
        return createCollisionGroup128(name);

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

void* ElemateResourceCallback::createCollisionGroup128(const std::string & name)
{
    physx::PxFilterData g;
    g.word0 = 3;
    g.word2 = ~0;
    g.word1 = g.word3 = 0;
    PX_ASSERT(m_apexSDK);
    PX_ASSERT(m_FilterDatas.size() < 128);
    // only set this one ressource for now...
    if (m_apexSDK && m_FilterDatas.empty())
    {
        m_FilterDatas.push_back(g);
        m_apexSDK->getNamedResourceProvider()->setResource(APEX_COLLISION_GROUP_128_NAME_SPACE, name.c_str(), (void*) &m_FilterDatas.back());
    }
    return (void*)&m_FilterDatas[0];
}


}
}
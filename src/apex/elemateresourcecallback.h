#pragma once

#include <string>
#include <vector>

#include <NxResourceCallback.h>
#include <PxFiltering.h>
#include <unordered_map>

#include <NxApexSDK.h>
namespace physx {
    class PxFilterData;
namespace apex {

class NxApexSDK;

class ElemateResourceCallback : public NxResourceCallback
{
public:
    ElemateResourceCallback();

    void setApexSDK(NxApexSDK * apexSDK);

    void* requestResource(const char* nameSpace, const char* name);
    void releaseResource(const char* nameSpace, const char* name, void* resource);


private:
    void* loadSingleResourceRaw(const std::string & name) const;

    std::unordered_map < std::string, physx::PxFilterData >   m_simulation_filter_data;
    std::unordered_map < std::string, physx::NxGroupsMask64 > m_group_masks;

    NxApexSDK*	m_apexSDK;
};

}
}

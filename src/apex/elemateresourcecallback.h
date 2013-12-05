#pragma once

#include <string>
#include <vector>

#include <NxResourceCallback.h>
#include <PxFiltering.h>

namespace physx {
namespace apex {

class NxApexSDK;

class ElemateResourceCallback : public NxResourceCallback
{
public:
    ElemateResourceCallback();

    void setApexSDK(NxApexSDK * apexSDK);

    void* requestResource(const char* nameSpace, const char* name);
    void releaseResource(const char* nameSpace, const char* name, void* resource);

    void* createCollisionGroup128(const std::string & name);

private:
    void* loadSingleResourceRaw(const std::string & name) const;

    std::vector<physx::PxFilterData> m_FilterDatas;

    NxApexSDK*	m_apexSDK;
};

}
}

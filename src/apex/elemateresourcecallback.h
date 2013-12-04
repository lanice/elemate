#pragma once

#include <string>

#include <NxResourceCallback.h>

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

private:
    void* loadSingleResourceRaw(const std::string & name) const;

    NxApexSDK*	m_apexSDK;
};

}
}

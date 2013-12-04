#pragma once

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
    NxApexSDK*	m_apexSDK;
};

}
}

#include "elemateresourcecallback.h"

#include <iostream>

#include <NxApexSDK.h>

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
    std::cerr << "Requested unknown ressource: " << nameSpace << "::" << name << std::endl;
    return nullptr;
}

void ElemateResourceCallback::releaseResource(const char* nameSpace, const char* name, void* resource)
{
    std::cerr << "Requested release of unknown ressource: " << nameSpace << "::" << name << " (at: " << resource << ")" << std::endl;
}


}
}
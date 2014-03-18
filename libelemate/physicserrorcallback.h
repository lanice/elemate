#pragma once

#include "utils/pxcompilerfix.h"
#include <foundation/PxErrorCallback.h>

class PhysicsErrorCallback : public physx::PxErrorCallback
{
public:
    /** return the last logged error code and reset it to eNO_ERROR */
    static physx::PxErrorCode::Enum getLastError();

private:
    /** only the PhysicsWrapper needs an error callback instance, that's why it has private access. */
    friend class PhysicsWrapper;

    virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override;
    static physx::PxErrorCode::Enum s_lastError;
};

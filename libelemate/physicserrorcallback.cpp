#include "physicserrorcallback.h"

#include "glow/logging.h"

physx::PxErrorCode::Enum PhysicsErrorCallback::s_lastError = physx::PxErrorCode::Enum::eNO_ERROR;

void PhysicsErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
{
    s_lastError = code;
    switch (code) {
    case physx::PxErrorCode::eNO_ERROR:
        glow::info("PhysX: ""%;"" [%;%;]", message, file, line);
        break;
    case physx::PxErrorCode::eDEBUG_INFO:
        glow::debug("PhysX: ""%;"" [%;%;]", message, file, line);
        break;
    case physx::PxErrorCode::eDEBUG_WARNING:
        glow::warning("PhysX: ""%;"" [%;%;]", message, file, line);
        break;
    case physx::PxErrorCode::eINVALID_PARAMETER:
    case physx::PxErrorCode::eINVALID_OPERATION:
    case physx::PxErrorCode::eOUT_OF_MEMORY:
        glow::critical("PhysX: ""%;"" [%;%;]", message, file, line);
        break;
    case physx::PxErrorCode::eINTERNAL_ERROR:
        glow::fatal("PhysX: ""%;"" [%;%;]", message, file, line);
        break;
    case physx::PxErrorCode::eABORT:
        glow::fatal("PhysX: ""%;"" [%;%;]", message, file, line);
        exit(2);
    case physx::PxErrorCode::ePERF_WARNING:
        glow::debug("PhysX (performance): ""%;"" [%;%;]", message, file, line);
        break;
    case physx::PxErrorCode::eMASK_ALL:
        glow::fatal("PhysX (something terrible happened): ""%;"" [%;%;]", message, file, line);
        break;
    }
}

physx::PxErrorCode::Enum PhysicsErrorCallback::getLastError()
{
    physx::PxErrorCode::Enum lastError = s_lastError;
    s_lastError = physx::PxErrorCode::Enum::eNO_ERROR;
    return lastError;
}

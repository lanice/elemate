#pragma once


#include "NxApex.h"
#include <vector>

namespace physx{
    namespace apex{
        class NxModuleParticleIos;
        class NxModuleEmitter;
        class NxModuleIofx;
        class NxApexRenderVolume;
    }
}

class StandardParticles
{
public:
    StandardParticles();
    ~StandardParticles();

    void initialize(physx::apex::NxApexSDK* gApexSDK);
    bool createEmitter(physx::apex::NxApexSDK* gApexSDK, physx::apex::NxApexScene* gApexScene);

    /** Should be used to render the particles. By writing a new Renderer that fits into our and the APEX Pipeline. */
    void renderVolume(physx::apex::NxUserRenderer & renderer);

protected:
    bool checkErrorCode(physx::apex::NxApexCreateError* err);

private:
    physx::apex::NxModuleParticleIos*   m_particle_ios_module;
    physx::apex::NxModuleEmitter*       m_emitter_module;
    physx::apex::NxModuleIofx*          m_iofx_module;
    physx::apex::NxApexRenderVolume*    m_render_volume;
};
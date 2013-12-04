#include "standardparticles.h"

#include <iostream>

#include "NxApexSDK.h"

#include "iofx/public/NxModuleIofx.h"
#include "iofx/public/NxIofxAsset.h"
#include <iofx/public/NxApexRenderVolume.h>

#include "emitter/public/NxModuleEmitter.h"
#include "emitter/public/NxApexEmitterAsset.h"
#include "emitter/public/NxApexEmitterActor.h"

#include "pxparticleios/public/NxModuleParticleIos.h"
#include "pxparticleios/public/NxParticleIosAsset.h"

#include "NxParamUtils.h"


StandardParticles::StandardParticles() :
    m_particle_ios_module(0),
    m_emitter_module(0),
    m_iofx_module(0)
{}

StandardParticles::~StandardParticles()
{}

void StandardParticles::initialize(physx::apex::NxApexSDK* gApexSDK)
{
    PX_ASSERT(gApexSDK);
    physx::apex::NxApexCreateError errorCode;


    m_particle_ios_module = static_cast<physx::apex::NxModuleParticleIos*>(gApexSDK->createModule("ParticleIOS", &errorCode));
    checkErrorCode(&errorCode);
    PX_ASSERT(m_particle_ios_module);
    if(m_particle_ios_module)
    {
        NxParameterized::Interface* params = m_particle_ios_module->getDefaultModuleDesc();
        m_particle_ios_module->init(*params);
    }

    
    m_iofx_module = static_cast<physx::apex::NxModuleIofx*>(gApexSDK->createModule("IOFX", &errorCode));
    checkErrorCode(&errorCode);
    PX_ASSERT(m_iofx_module);
    if (m_iofx_module)
    {
        NxParameterized::Interface* params = m_iofx_module->getDefaultModuleDesc();
        m_iofx_module->init(*params);
    }

    m_emitter_module = static_cast<physx::apex::NxModuleEmitter*> (gApexSDK->createModule("Emitter", &errorCode));
    checkErrorCode(&errorCode);
    PX_ASSERT(m_emitter_module);
    if(m_emitter_module)
    {
        NxParameterized::Interface* params = m_emitter_module->getDefaultModuleDesc();
        m_emitter_module->init(*params);
        physx::PxU32 numScalables = m_emitter_module->getNbParameters();
        physx::apex::NxApexParameter** m_emitterModuleScalables = m_emitter_module->getParameters();
        for (physx::PxU32 i = 0; i < numScalables; i++)
        {
            physx::apex::NxApexParameter& p = *m_emitterModuleScalables[i];
            m_emitter_module->setIntValue(i, p.range.maximum);
        }
    }

}

bool StandardParticles::createEmitter(physx::apex::NxApexSDK* gApexSDK, physx::apex::NxApexScene* gApexScene)
{
    physx::apex::NxResourceProvider * provider = gApexSDK->getNamedResourceProvider();
    physx::apex::NxApexAsset* asset = reinterpret_cast<physx::apex::NxApexAsset*>(provider->getResource(NX_APEX_EMITTER_AUTHORING_TYPE_NAME, "testSpriteEmitter4ParticleFluidIos"));
    if (asset == nullptr) {
        std::cerr << "Failed to create the APEX Emitter Asset" << std::endl;
        return false;
    }
    physx::apex::NxApexEmitterAsset* emitterAsset = static_cast<physx::apex::NxApexEmitterAsset*> (asset);
    //NxApexEmitterAsset* emitterAsset = static_cast<NxApexEmitterAsset*> (gApexSDK->createAsset(asParams, "testMeshEmitter4ParticleIos.apb"));
    gApexSDK->forceLoadAssets();

    NxParameterized::Interface* descParams = emitterAsset->getDefaultActorDesc();
    PX_ASSERT(descParams);

    // Set Actor pose
    //NxParameterized::setParamMat44( *descParams, "initialPose", pose );
    physx::apex::NxApexEmitterActor* emitterActor;
    emitterActor = static_cast<physx::apex::NxApexEmitterActor*>(emitterAsset->createApexActor(*descParams, *gApexScene));
    emitterActor->setCurrentPosition(physx::PxVec3(0.0f, 1.0f, 0.0f));
    emitterActor->startEmit( true );
    //emitterActor->setLifetimeRange(physx::apex::NxRange<PxF32>(1,5));
    //emitterActor->setRateRange(physx::apex::NxRange<PxF32>(10, 10));
     
    physx::PxBounds3 b;
    b.setInfinite();

    m_render_volume = m_iofx_module->createRenderVolume(*gApexScene, b, 0, true );
    emitterActor->setPreferredRenderVolume( m_render_volume );

    return true;
}

void StandardParticles::renderVolume(physx::apex::NxUserRenderer & renderer)
{
    m_render_volume->lockRenderResources();
  
    m_render_volume->updateRenderResources(false);
    m_render_volume->dispatchRenderResources(renderer);
    
	m_render_volume->unlockRenderResources();
}

bool StandardParticles::checkErrorCode(physx::apex::NxApexCreateError* err)
{
    if (*err == physx::apex::APEX_CE_NO_ERROR)
        return true;

    std::cerr << "Apex error: ";
    switch (*err) {
    case physx::apex::APEX_CE_NOT_FOUND:
        std::cout << "Unable to find the libraries." << std::endl;
        break;
    case physx::apex::APEX_CE_WRONG_VERSION:
        std::cout << "The application supplied a version number that does not match with the libraries." << std::endl;
        break;
    case physx::apex::APEX_CE_DESCRIPTOR_INVALID:
        std::cout << "The supplied descriptor is invalid." << std::endl;
        break;
    }
    return false;
}
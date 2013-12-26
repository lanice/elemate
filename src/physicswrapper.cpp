#include "physicswrapper.h"

#include <iostream>
#include <cassert>

#include <glow/logging.h>

#include "elements.h"
#include "particleemitter.h"
// #include "particledrawable.h"


const int   PhysicsWrapper::kNumberOfThreads = 2;

PhysicsWrapper::PhysicsWrapper():
        m_foundation(nullptr),
        m_physics(nullptr),
        //m_profile_zone_manager(nullptr),
        m_scene(nullptr),
        m_cpu_dispatcher(nullptr)
{
    initializePhysics();
    initializeScene();
    Elements::initialize(*m_physics);
}

PhysicsWrapper::~PhysicsWrapper(){
    m_scene->fetchResults(); //Wait for last simulation step to complete before releasing scene
    m_scene->release();
    m_physics->release();
    m_cpu_dispatcher->release();
    //Please don't forget if you activate this feature.
    //m_profile_zone_manager->release();
    m_foundation->release();
}

bool PhysicsWrapper::step(long double delta){

    if (delta == 0)
        return false;
    
    m_scene->simulate(static_cast<physx::PxReal>(delta));
    // m_scene->fetchResults();
    updateAllObjects(delta);
    
    return true;
}

void PhysicsWrapper::updateAllObjects(long double delta)
{
    m_scene->fetchResults(true);

    for (auto& emitter : m_emitters){
        emitter->update(delta);
    }

    physx::PxMat44 new_pos;

    static bool here = false;
    if (!here) {
        glow::debug("TODO: ObjectsContainer::updateAllObjects()");
        here = true;
    }

    /*for (auto& current_object : m_objects){
        if (current_object.second->isSleeping())
            continue;*/

        /*new_pos = physx::PxMat44(current_object.second->getGlobalPose());
        osg::Matrix newTransform = convertPxMat44ToOsgMatrix(new_pos);
        current_object.first->setMatrix(newTransform);

        osg::Vec3d translation; osg::Quat rotation; osg::Vec3d scale; osg::Quat scaleorientation;
        newTransform.decompose(translation, rotation, scale, scaleorientation);
        current_object.first->getOrCreateStateSet()->getOrCreateUniform("modelRotation",
            osg::Uniform::Type::FLOAT_MAT4)->set(osg::Matrixf(rotation));
    }*/
}

void PhysicsWrapper::makeParticleEmitter(const physx::PxVec3& position){
    m_emitters.push_back(new ParticleEmitter(position));
    m_emitters.back()->initializeParticleSystem();
    m_emitters.back()->startEmit();
}

void PhysicsWrapper::makeStandardBall(const physx::PxVec3& /*global_position*/, physx::PxReal /*radius*/, const physx::PxVec3& /*linear_velocity*/, const physx::PxVec3& /*angular_velocity*/)
{
    glm::mat4 translation;

    assert(false);
    /*translation.setTrans(osg::Vec3(global_position.x, global_position.y, global_position.z));
    osg::ref_ptr<osg::MatrixTransform> osgTransformNode = new osg::MatrixTransform(translation);
    
    osg::ref_ptr<osg::Geode> sphere_geode = new osg::Geode();
    sphere_geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0,0,0), radius)));
    osgTransformNode->addChild(sphere_geode);
    parent->addChild(osgTransformNode.get());*/

    /*auto physx_object = PxCreateDynamic(PxGetPhysics(), physx::PxTransform(global_position), physx::PxSphereGeometry(radius), *Elements::pxMaterial("default"), 1.0F);
    physx_object->setLinearVelocity(linear_velocity);
    physx_object->setAngularVelocity(angular_velocity);
    m_physics_wrapper->scene()->addActor(*physx_object);*/

    //m_objects.push_back(DrawableAndPhysXObject(osgTransformNode.get(), physx_object));
}

void PhysicsWrapper::initializePhysics(){
    static physx::PxDefaultErrorCallback gDefaultErrorCallback;
    static physx::PxDefaultAllocator     gDefaultAllocatorCallback;

    m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
    if (!m_foundation)
        fatalError("PxCreateFoundation failed!");
    m_physics= PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, physx::PxTolerancesScale());
    if (!m_physics)
        fatalError("PxCreatePhysics failed!");

    /* ... we still have to think about those:
    //For Debugging Lab ....
    m_profile_zone_manager = &physx::PxProfileZoneManager::createProfileZoneManager(m_foundation);
    if (!m_profile_zone_manager)
        fatalError("PxProfileZoneManager::createProfileZoneManager failed!");
    
    // Cooking
    // The PhysX cooking library provides utilities for creating, converting, and serializing bulk data. 
    // Depending on your application, you may wish to link to the cooking library in order to process such data at runtime. 
    // Alternatively you may be able to process all such data in advance and just load it into memory as required. 
    // Initialize the cooking library as follows: (after declaring member physx::PxCooking* m_cooking)

    m_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_foundation, PxCookingParams());
    if (!m_cooking)
        fatalError("PxCreateCooking failed!");
    */

    //    Extensions
    // The extensions library contains many functions that may be useful to a large class of users, 
    // but which some users may prefer to omit from their application either for code size reasons or 
    // to avoid use of certain subsystems, such as those pertaining to networking. 
    // Initializing the extensions library requires the PxPhysics object:
    if (!PxInitExtensions(*m_physics))
        fatalError("PxInitExtensions failed!");
}

void PhysicsWrapper::initializeScene(){
    physx::PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
    customizeSceneDescription(sceneDesc);

    if (!sceneDesc.cpuDispatcher)
    {
        m_cpu_dispatcher = physx::PxDefaultCpuDispatcherCreate(kNumberOfThreads);
        if (!m_cpu_dispatcher)
            fatalError("PxDefaultCpuDispatcherCreate failed!");
        sceneDesc.cpuDispatcher = m_cpu_dispatcher;
    }
    if (!sceneDesc.filterShader)
        sceneDesc.filterShader = &physx::PxDefaultSimulationFilterShader;

    m_scene = m_physics->createScene(sceneDesc);
    if (!m_scene)
        fatalError("createScene failed!");
}

void PhysicsWrapper::customizeSceneDescription(physx::PxSceneDesc& scene_description){
    scene_description.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
}

void PhysicsWrapper::fatalError(std::string error_message){
    std::cerr << "PhysX Error occured:" << std::endl;
    std::cerr << error_message << std::endl;
    std::cerr << "Press Enter to close the Application" << std::endl;
    std::string temp;
    std::getline(std::cin, temp);
    exit(1);
}

physx::PxScene* PhysicsWrapper::scene()const{
    return m_scene;
}

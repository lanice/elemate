#include "objectscontainer.h"

#include <cassert>
#include <iostream>

#include <osg/ShapeDrawable>
#include <osgViewer/View>
#include <osg/MatrixTransform>
#include <osg/Point>
#include "particleemitter.h"
#include "elements.h"
#include "physicswrapper.h"
#include "particledrawable.h"

ObjectsContainer::ObjectsContainer(std::shared_ptr<PhysicsWrapper> physics_wrapper) :
    m_physics_wrapper(physics_wrapper)
{
}

ObjectsContainer::~ObjectsContainer()
{
    m_objects.clear();
}

inline osg::Matrix convertPxMat44ToOsgMatrix(const physx::PxMat44 px_mat){
    return osg::Matrix(
        px_mat(0, 0), px_mat(1, 0), px_mat(2, 0), px_mat(3, 0),
        px_mat(0, 1), px_mat(1, 1), px_mat(2, 1), px_mat(3, 1),
        px_mat(0, 2), px_mat(1, 2), px_mat(2, 2), px_mat(3, 2),
        px_mat(0, 3), px_mat(1, 3), px_mat(2, 3), px_mat(3, 3)
        );
}

void ObjectsContainer::updateAllObjects()
{
    m_physics_wrapper->scene()->fetchResults(true);

    for (auto& emitter : m_emitters){
        emitter->update(m_physics_wrapper->elapsedTime());
    }

    physx::PxMat44 new_pos;
    for (auto& current_object : m_objects){
        if (current_object.second->isSleeping())
            continue;

        new_pos = physx::PxMat44(current_object.second->getGlobalPose());
        osg::Matrix newTransform = convertPxMat44ToOsgMatrix(new_pos);
        current_object.first->setMatrix(newTransform);

        osg::Vec3d translation; osg::Quat rotation; osg::Vec3d scale; osg::Quat scaleorientation;
        newTransform.decompose(translation, rotation, scale, scaleorientation);
        current_object.first->getOrCreateStateSet()->getOrCreateUniform("modelRotation",
            osg::Uniform::Type::FLOAT_MAT4)->set(osg::Matrixf(rotation));
    }
}

void ObjectsContainer::makeStandardBall(osg::ref_ptr<osg::Group> parent, const physx::PxVec3& global_position, physx::PxReal radius, const physx::PxVec3& linear_velocity, const physx::PxVec3& angular_velocity)
{
    osg::Matrix translation;
    translation.setTrans(osg::Vec3(global_position.x, global_position.y, global_position.z));
    osg::ref_ptr<osg::MatrixTransform> osgTransformNode = new osg::MatrixTransform(translation);
    
    osg::ref_ptr<osg::Geode> sphere_geode = new osg::Geode();
    sphere_geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0,0,0), radius)));
    osgTransformNode->addChild(sphere_geode);
    parent->addChild(osgTransformNode.get());

    auto physx_object = PxCreateDynamic(PxGetPhysics(), physx::PxTransform(global_position), physx::PxSphereGeometry(radius), *Elements::pxMaterial("default"), 1.0F);
    physx_object->setLinearVelocity(linear_velocity);
    physx_object->setAngularVelocity(angular_velocity);
    m_physics_wrapper->scene()->addActor(*physx_object);

    m_objects.push_back(DrawableAndPhysXObject(osgTransformNode.get(), physx_object));
}

void ObjectsContainer::makeParticleEmitter(osg::ref_ptr<osg::Group> parent, const physx::PxVec3& position){
    m_emitters.push_back(new ParticleEmitter(parent, position));
    m_emitters.back()->initializeParticleSystem();
    m_emitters.back()->startEmit();
}

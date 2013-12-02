#include "objectscontainer.h"
#include "physicswrapper.h"

#include <osg/ShapeDrawable>
#include <osgViewer/View>
#include <osg/MatrixTransform>

ObjectsContainer::ObjectsContainer(std::shared_ptr<PhysicsWrapper> physics_wrapper) :
    m_physics_wrapper(physics_wrapper)
{}

ObjectsContainer::~ObjectsContainer()
{
    m_objects.clear();
}

void ObjectsContainer::updateAllObjects(){
    m_physics_wrapper->scene()->fetchResults();
    
    physx::PxMat44 new_pos;
    for (auto& current_object : m_objects){
        new_pos = physx::PxMat44(current_object.second->getGlobalPose());
        osg::Matrix newTransform(
            new_pos(0, 0), new_pos(1, 0), new_pos(2, 0), new_pos(3, 0),
            new_pos(0, 1), new_pos(1, 1), new_pos(2, 1), new_pos(3, 1),
            new_pos(0, 2), new_pos(1, 2), new_pos(2, 2), new_pos(3, 2),
            new_pos(0, 3), new_pos(1, 3), new_pos(2, 3), new_pos(3, 3)
            );
        current_object.first->setMatrix(newTransform);
        osg::Vec3d translation; osg::Quat rotation; osg::Vec3d scale; osg::Quat scaleorientation;
        newTransform.decompose(translation, rotation, scale, scaleorientation);
        current_object.first->getOrCreateStateSet()->getOrCreateUniform("modelRotation",
            osg::Uniform::Type::FLOAT_MAT4)->set(osg::Matrixf(rotation));
    }
}

void ObjectsContainer::makeStandardBall(osg::ref_ptr<osg::Group> parent, const physx::PxVec3& global_position, physx::PxReal radius, const physx::PxVec3& linear_velocity, const physx::PxVec3& angular_velocity){
    auto osg_object = new osg::MatrixTransform();
    osg::ref_ptr<osg::Geode> sphere_geode = new osg::Geode();
    sphere_geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0,0,0), radius)));
    osg_object->addChild(sphere_geode);
    parent->addChild(osg_object);

    auto physx_object = PxCreateDynamic(PxGetPhysics(), physx::PxTransform(global_position), physx::PxSphereGeometry(radius), *m_physics_wrapper->material("default"), 1.0F);
    physx_object->setLinearVelocity(linear_velocity);
    physx_object->setAngularVelocity(angular_velocity);
    m_physics_wrapper->scene()->addActor(*physx_object);

    m_objects.push_back(DrawableAndPhysXObject(osg_object, physx_object));
}

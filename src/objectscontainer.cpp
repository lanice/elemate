#include "objectscontainer.h"
#include "physicswrapper.h"
#include "worlddrawable.h"

#include <osg/ShapeDrawable>
#include <osgViewer/View>
#include <osg/MatrixTransform>

ObjectsContainer::ObjectsContainer(std::shared_ptr<PhysicsWrapper> physics_wrapper) :
    m_physics_wrapper(physics_wrapper)
{}

ObjectsContainer::~ObjectsContainer()
{}

void ObjectsContainer::updateAllObjects(){
    m_physics_wrapper->scene()->fetchResults();

    physx::PxMat44 new_pos(m_sphere1.second->getGlobalPose());
    m_sphere1.first->setMatrix(
        osg::Matrix(
        new_pos(0, 0), new_pos(1, 0), new_pos(2, 0), new_pos(3, 0),
        new_pos(0, 1), new_pos(1, 1), new_pos(2, 1), new_pos(3, 1),
        new_pos(0, 2), new_pos(1, 2), new_pos(2, 2), new_pos(3, 2),
        new_pos(0, 3), new_pos(1, 3), new_pos(2, 3), new_pos(3, 3)
        )
    );


    new_pos = physx::PxMat44(m_sphere2.second->getGlobalPose());
    m_sphere2.first->setMatrix(
        osg::Matrix(
        new_pos(0, 0), new_pos(1, 0), new_pos(2, 0), new_pos(3, 0),
        new_pos(0, 1), new_pos(1, 1), new_pos(2, 1), new_pos(3, 1),
        new_pos(0, 2), new_pos(1, 2), new_pos(2, 2), new_pos(3, 2),
        new_pos(0, 3), new_pos(1, 3), new_pos(2, 3), new_pos(3, 3)
        )
    );
}

void ObjectsContainer::makeBall(osg::ref_ptr<osg::Group> parent){
    // Creates a Sphere
    //OSG Object
    m_sphere1.first = new osg::MatrixTransform();
    osg::ref_ptr<osg::Geode> sphere1_geode = new osg::Geode();
    sphere1_geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0, 0, 0), 0.2)));
    m_sphere1.first->addChild(sphere1_geode);
    parent->addChild(m_sphere1.first);
    //PhysX Object
    m_sphere1.second = PxCreateDynamic(PxGetPhysics(), physx::PxTransform(physx::PxVec3(1, 3, 0)), physx::PxSphereGeometry(0.2F), *m_physics_wrapper->material("default"), 1.0F);
    m_sphere1.second->setLinearVelocity(physx::PxVec3(-2, 4.0, 0));
    m_sphere1.second->setAngularVelocity(physx::PxVec3(6.0, 13.0, 1.0));
    m_physics_wrapper->scene()->addActor(*m_sphere1.second);

    // Creates a Sphere
    //OSG Object
    m_sphere2.first = new osg::MatrixTransform();
    osg::ref_ptr<osg::Geode> sphere2_geode = new osg::Geode();
    sphere2_geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0, 0, 0), 0.2)));
    m_sphere2.first->addChild(sphere2_geode);
    parent->addChild(m_sphere2.first);
    //PhysX Object
    m_sphere2.second = PxCreateDynamic(PxGetPhysics(), physx::PxTransform(physx::PxVec3(-1, 3, 0)), physx::PxSphereGeometry(0.2F), *m_physics_wrapper->material("default"), 1.0F);
    m_sphere2.second->setLinearVelocity(physx::PxVec3(2, 4.0, 0));
    m_physics_wrapper->scene()->addActor(*m_sphere2.second);
}

void ObjectsContainer::makePlane(osg::ref_ptr<osg::Group> parent){
    //Creates a plane
    //OSG Object
    osg::ref_ptr<WorldDrawable> world = new WorldDrawable;
    osg::ref_ptr<osg::Geode> world_geode = new osg::Geode();
    world_geode->addDrawable(world.get());
    parent->addChild(world_geode.get());
    //PhysXObject
    physx::PxRigidStatic* plane = PxCreatePlane(PxGetPhysics(), physx::PxPlane(physx::PxVec3(0, 1, 0), 0.0F), *m_physics_wrapper->material("default"));
    m_physics_wrapper->scene()->addActor(*plane);
}
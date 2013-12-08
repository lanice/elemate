#include "objectscontainer.h"
#include "physicswrapper.h"

#include <osg/ShapeDrawable>
#include <osgViewer/View>
#include <osg/MatrixTransform>



ObjectsContainer::ObjectsContainer(std::shared_ptr<PhysicsWrapper> physics_wrapper) :
    m_physics_wrapper(physics_wrapper)
{
    initializeParticles();
}

ObjectsContainer::~ObjectsContainer()
{
    m_objects.clear();
    m_particle_system->releaseParticles();
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
    physx::PxMat44 new_pos;
    for (auto& current_object : m_objects){
        new_pos = physx::PxMat44(current_object.second->getGlobalPose());
        osg::Matrix newTransform = convertPxMat44ToOsgMatrix(new_pos);
        current_object.first->setMatrix(newTransform);

        osg::Vec3d translation; osg::Quat rotation; osg::Vec3d scale; osg::Quat scaleorientation;
        newTransform.decompose(translation, rotation, scale, scaleorientation);
        current_object.first->getOrCreateStateSet()->getOrCreateUniform("modelRotation",
            osg::Uniform::Type::FLOAT_MAT4)->set(osg::Matrixf(rotation));
    }
    auto read_data = m_particle_system->lockParticleReadData();
    if (!read_data)
        return;
    auto physx_pos_iter = read_data->positionBuffer;
    auto  osg_pos_iter = m_particle_objects.begin();
    physx::PxU32 particle_count = kMaxParticleCount;

    while (--particle_count)
    {
        if (!physx_pos_iter.ptr() || osg_pos_iter == m_particle_objects.end())
            return;
        auto px_mat = *physx_pos_iter.ptr();
        osg::Matrix newTransform = osg::Matrix( 1, 0, 0, 0,
                                                0, 1, 0, 0,
                                                0, 0, 1, 0,
                                                px_mat.x, px_mat.y, px_mat.z, 1);
        
        (*osg_pos_iter)->setMatrix(newTransform);
        osg::Vec3d translation; osg::Quat rotation; osg::Vec3d scale; osg::Quat scaleorientation;
        newTransform.decompose(translation, rotation, scale, scaleorientation);
        (*osg_pos_iter)->getOrCreateStateSet()->getOrCreateUniform("modelRotation",
        osg::Uniform::Type::FLOAT_MAT4)->set(osg::Matrixf(rotation));
        
        physx_pos_iter++;
        osg_pos_iter++;
    }
    read_data->unlock();
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

    auto physx_object = PxCreateDynamic(PxGetPhysics(), physx::PxTransform(global_position), physx::PxSphereGeometry(radius), *m_physics_wrapper->material("default"), 1.0F);
    physx_object->setLinearVelocity(linear_velocity);
    physx_object->setAngularVelocity(angular_velocity);
    m_physics_wrapper->scene()->addActor(*physx_object);

    m_objects.push_back(DrawableAndPhysXObject(osgTransformNode.get(), physx_object));
}

void ObjectsContainer::initializeParticles(){
    m_particle_system = PxGetPhysics().createParticleSystem(kMaxParticleCount, false);

    if (m_particle_system)
        m_physics_wrapper->scene()->addActor(*m_particle_system);
}

void ObjectsContainer::createParticleObject(osg::ref_ptr<osg::Group> parent, const physx::PxVec3& position){
    osg::Matrix translation;
    translation.setTrans(osg::Vec3(position.x, position.y, position.z));
    osg::ref_ptr<osg::MatrixTransform> osgTransformNode = new osg::MatrixTransform(translation);

    osg::ref_ptr<osg::Geode> sphere_geode = new osg::Geode();
    sphere_geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0, 0, 0), 0.1)));
    osgTransformNode->addChild(sphere_geode);
    parent->addChild(osgTransformNode.get());

    m_particle_objects.push_back(osgTransformNode.get());
}

void ObjectsContainer::createParticles(osg::ref_ptr<osg::Group> parent, int number_of_particles, const physx::PxVec3& position){
    if (number_of_particles > kMaxParticleCount)
        number_of_particles = kMaxParticleCount;

    physx::PxParticleCreationData particleCreationData;
    particleCreationData.numParticles = number_of_particles;
    physx::PxU32    m_particle_index_buffer[kMaxParticleCount];
    physx::PxVec3   m_particle_position_buffer[kMaxParticleCount];
    physx::PxVec3   m_particle_velocity_buffer[kMaxParticleCount];

    size_t pre_existing_particles = m_particle_objects.size();
    for (size_t i = 0; i < number_of_particles; i++)
    {
        m_particle_index_buffer[i] = (pre_existing_particles + i) % kMaxParticleCount;
        m_particle_position_buffer[i] = position;
        m_particle_velocity_buffer[i] = physx::PxVec3(0.0F + (rand() % 4 - 2), 0.0F + (rand() % 4 - 2), 0.0F + (rand() % 4 - 2));
        createParticleObject(parent, position);
    }

    particleCreationData.indexBuffer = physx::PxStrideIterator<const physx::PxU32>(m_particle_index_buffer);
    particleCreationData.positionBuffer = physx::PxStrideIterator<const physx::PxVec3>(m_particle_position_buffer);
    particleCreationData.velocityBuffer = physx::PxStrideIterator<const physx::PxVec3>(m_particle_velocity_buffer);

    m_particle_system->createParticles(particleCreationData);
}
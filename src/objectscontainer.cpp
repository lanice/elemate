#include "objectscontainer.h"

#include <cassert>
#include <iostream>

#include <osg/ShapeDrawable>
#include <osgViewer/View>
#include <osg/MatrixTransform>
#include <osgUtil/GLObjectsVisitor>

#include "elements.h"
#include "physicswrapper.h"

ObjectsContainer::ObjectsContainer(std::shared_ptr<PhysicsWrapper> physics_wrapper) :
    m_physics_wrapper(physics_wrapper)
{
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
    m_physics_wrapper->scene()->fetchResults(true);
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
    physx::PxParticleReadData * read_data = m_particle_system->lockParticleReadData();
    assert(read_data);

    auto physx_pos_iter = read_data->positionBuffer;
    osg::ref_ptr<osg::Geometry> waterGeometry = m_particle_geometries.at("water");

    osg::ref_ptr<osg::Vec3Array> waterVertices = dynamic_cast<osg::Vec3Array*>(waterGeometry->getVertexArray());

    assert(waterVertices.valid());
    assert(kMaxParticleCount == waterVertices->size());
    auto osg_pos_iter = waterVertices->begin();

    for (; osg_pos_iter != waterVertices->end() && physx_pos_iter.ptr(); ++physx_pos_iter, ++osg_pos_iter)
    {
        assert(physx_pos_iter.ptr());
        const physx::PxVec3 & px_pos = *physx_pos_iter.ptr();
        
        osg_pos_iter->set(px_pos.x, px_pos.y, px_pos.z);
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

    auto physx_object = PxCreateDynamic(PxGetPhysics(), physx::PxTransform(global_position), physx::PxSphereGeometry(radius), *Elements::pxMaterial("default"), 1.0F);
    physx_object->setLinearVelocity(linear_velocity);
    physx_object->setAngularVelocity(angular_velocity);
    m_physics_wrapper->scene()->addActor(*physx_object);

    m_objects.push_back(DrawableAndPhysXObject(osgTransformNode.get(), physx_object));
}

void ObjectsContainer::initializeParticles(osg::Group * particleGroup){
    m_particle_group = particleGroup;

    m_particle_system = PxGetPhysics().createParticleSystem(kMaxParticleCount, false);
    assert(m_particle_system);

    physx::PxScene* scene_buffer = static_cast<physx::PxScene*>(malloc(sizeof(physx::PxScene)));
    PxGetPhysics().getScenes(&scene_buffer,1);
    scene_buffer->addActor(*m_particle_system);


    osg::ref_ptr<osg::Geometry> waterGeometry = new osg::Geometry();
    waterGeometry->setVertexArray(new osg::Vec3Array(kMaxParticleCount));
    waterGeometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, 0));
    waterGeometry->setUseVertexBufferObjects(true);
    m_particle_geometries.emplace("water", waterGeometry.get());

    osg::ref_ptr<osg::Geode> particleGeode = new osg::Geode;
    particleGeode->addDrawable(waterGeometry.get());

    assert(m_particle_group.valid());
    m_particle_group->addChild(particleGeode.get());

    
        
    /*auto drain_plane = PxCreatePlane(PxGetPhysics(), physx::PxPlane(physx::PxVec3(0.0F, 1.0F, 0.0F), 15.0F), *Elements::pxMaterial("default"));
    physx::PxShape* shape = drain_plane->createShape(physx::PxPlaneGeometry(), *Elements::pxMaterial("default"));

    shape->setFlag(physx::PxShapeFlag::ePARTICLE_DRAIN, true);
    m_physics_wrapper->scene()->addActor(*drain_plane);*/
}

void ObjectsContainer::createParticles(unsigned int number_of_particles, const physx::PxVec3& position)
{
    assert(m_particle_group.valid());

    if (number_of_particles > kMaxParticleCount)
        number_of_particles = kMaxParticleCount;

    physx::PxParticleCreationData particleCreationData;
    particleCreationData.numParticles = number_of_particles;
    physx::PxU32    m_particle_index_buffer[kMaxParticleCount];
    physx::PxVec3   m_particle_position_buffer[kMaxParticleCount];
    physx::PxVec3   m_particle_velocity_buffer[kMaxParticleCount];

    static size_t youngest_particle_index = 0;
    for (size_t i = 0; i < number_of_particles; i++)
    {
        m_particle_index_buffer[i] = (youngest_particle_index + i) % kMaxParticleCount;
        m_particle_position_buffer[i] = position;
        m_particle_velocity_buffer[i] = physx::PxVec3(0.0F + (rand() % 4 - 2), 0.0F + (rand() % 4 - 2), 0.0F + (rand() % 4 - 2));
    }

    particleCreationData.indexBuffer = physx::PxStrideIterator<const physx::PxU32>(m_particle_index_buffer);
    particleCreationData.positionBuffer = physx::PxStrideIterator<const physx::PxVec3>(m_particle_position_buffer);
    particleCreationData.velocityBuffer = physx::PxStrideIterator<const physx::PxVec3>(m_particle_velocity_buffer);

    assert(m_particle_system->createParticles(particleCreationData));



    // create associated osg/opengl buffers
    osg::ref_ptr<osg::Geometry> waterGeometry = m_particle_geometries.at("water").get();
    waterGeometry->releaseGLObjects();
    osg::ref_ptr<osg::Vec3Array> waterVertices = dynamic_cast<osg::Vec3Array*>(waterGeometry->getVertexArray());
    assert(waterVertices);
    assert(waterGeometry->getPrimitiveSetList().size() == 1);
    osg::ref_ptr<osg::DrawArrays> waterArrayDefs = dynamic_cast<osg::DrawArrays*>(waterGeometry->getPrimitiveSet(0));
    assert(waterArrayDefs.valid());

    // copy vertex data to osg
    for (unsigned int i = 0; i < kMaxParticleCount; ++i) {
        physx::PxVec3 & vertex = m_particle_position_buffer[i];
        waterVertices->at(i) = osg::Vec3(vertex.x, vertex.y, vertex.z);
    }

    unsigned newCount = waterArrayDefs->getCount() + number_of_particles;
    if (newCount >= kMaxParticleCount)
        newCount = kMaxParticleCount;
    waterArrayDefs->setCount(newCount);

    /*for (unsigned int i = 0; i < number_of_particles; ++i) {
        waterIndices->at(i) = (youngest_particle_index + i) % kMaxParticleCount;
    }*/

    youngest_particle_index = (youngest_particle_index + number_of_particles) % kMaxParticleCount;

    //osg::Geometry * geo = new osg::Geometry();
    //osg::Vec3Array * vec = new osg::Vec3Array();
    //vec->push_back(osg::Vec3(1, 3, 4));
    //vec->push_back(osg::Vec3(1, 6, 4));
    //vec->push_back(osg::Vec3(3, 3, 4));
    //geo->setVertexArray(vec);
    //geo->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 1, 2));
    //auto * geod = new osg::Geode();
    //geod->addDrawable(geo);
    //m_particle_group->addChild(geod);
}

void ObjectsContainer::makeParticleEmitter(osg::ref_ptr<osg::Group> parent, const physx::PxVec3& position){
    //createParticles(parent, 5, position);
}

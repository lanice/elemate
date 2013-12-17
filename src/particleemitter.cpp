#include "particleemitter.h"
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osgViewer/View>

ParticleEmitter::ParticleEmitter(osg::ref_ptr<osg::Group> parent, const physx::PxVec3& position):
    m_parent(parent),
    m_position(position),
    m_emitting(false),
    m_particles_per_second(kDefaultEmittedParticles)
{
}
ParticleEmitter::ParticleEmitter(osg::ref_ptr<osg::Group> parent):
    ParticleEmitter(parent,physx::PxVec3(0,0,0))
{
}

ParticleEmitter::~ParticleEmitter(){
    m_particle_system->releaseParticles();
}

void ParticleEmitter::initializeParticleSystem(){
    m_particle_system = PxGetPhysics().createParticleSystem(kMaxParticleCount, false);

    if (m_particle_system){
        physx::PxScene* scene_buffer = static_cast<physx::PxScene*>(malloc(sizeof(physx::PxScene)));
        PxGetPhysics().getScenes(&scene_buffer, 1);
        scene_buffer->addActor(*m_particle_system);
    }
}

void ParticleEmitter::update(t_longf elapsed_Time){
    static t_longf akkumulator = 1.0f/60.0f;
    if (akkumulator > 0 && m_emitting){
        akkumulator -= elapsed_Time;
        return;
    } else {
        akkumulator = 1.0f / 60.0f;

        createParticles(m_particles_per_second);
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
            break;
        auto px_mat = *physx_pos_iter.ptr();
        osg::Matrix newTransform = osg::Matrix(1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            px_mat.x, px_mat.y, px_mat.z, 1);

        (*osg_pos_iter)->setMatrix(newTransform);

        /*osg::Vec3d translation; osg::Quat rotation; osg::Vec3d scale; osg::Quat scaleorientation;
        newTransform.decompose(translation, rotation, scale, scaleorientation);
        (*osg_pos_iter)->getOrCreateStateSet()->getOrCreateUniform("modelRotation",
        osg::Uniform::Type::FLOAT_MAT4)->set(osg::Matrixf(rotation));*/

        physx_pos_iter++;
        osg_pos_iter++;
    }
    read_data->unlock();
}

void ParticleEmitter::startEmit(){
    m_emitting = true;
}

void ParticleEmitter::createParticles(int number_of_particles){
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
        m_particle_position_buffer[i] = m_position;
        m_particle_velocity_buffer[i] = physx::PxVec3(0.0F + (rand() % 4 - 2), 0.0F + (rand() % 4 - 2), 0.0F + (rand() % 4 - 2));
        createParticleObjects();
    }
    youngest_particle_index = (youngest_particle_index + number_of_particles) % kMaxParticleCount;

    particleCreationData.indexBuffer = physx::PxStrideIterator<const physx::PxU32>(m_particle_index_buffer);
    particleCreationData.positionBuffer = physx::PxStrideIterator<const physx::PxVec3>(m_particle_position_buffer);
    particleCreationData.velocityBuffer = physx::PxStrideIterator<const physx::PxVec3>(m_particle_velocity_buffer);

    m_particle_system->createParticles(particleCreationData);
}

void ParticleEmitter::createParticleObjects(){
    if (m_particle_objects.size() >= kMaxParticleCount)
        return;
    osg::Matrix translation;
    translation.setTrans(osg::Vec3(m_position.x, m_position.y, m_position.z));
    osg::ref_ptr<osg::MatrixTransform> osgTransformNode = new osg::MatrixTransform(translation);


    osg::ref_ptr<osg::Geode> sphere_geode = new osg::Geode();


    auto geo = new osg::Geometry();
    osg::Vec3Array* vertices = new osg::Vec3Array();
    vertices->push_back(osg::Vec3(0, 0.1, 0));
    vertices->push_back(osg::Vec3(0.05, 0, 0));
    vertices->push_back(osg::Vec3(-0.05, 0, 0));
    geo->setVertexArray(vertices);
    osg::DrawElementsUInt* triangle =
        new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
    triangle->push_back(2);
    triangle->push_back(1);
    triangle->push_back(0);
    geo->addPrimitiveSet(triangle);

    sphere_geode->addDrawable(geo);
    //sphere_geode->addDrawable(new osg::ShapeDrawable(new osg::Box(osg::Vec3(0, 0, 0), 0.25)));
    osgTransformNode->addChild(sphere_geode);
    m_parent->addChild(osgTransformNode.get());

    m_particle_objects.push_back(osgTransformNode.get());
}

void ParticleEmitter::stopEmit(){
    m_emitting = false;
}


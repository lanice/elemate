#pragma once


#define DISALLOW_COPY_AND_ASSIGN(TypeName)  \
    TypeName(const TypeName&);              \
    void operator=(const TypeName&);


#include <list>
#include <memory>
#include "PxPhysicsAPI.h"
#include <osg/GraphicsContext> // ref_ptr

namespace osg {   
    class MatrixTransform;
    class Group; 
}

class PhysicsWrapper;


/** This Class contains all (primitive) OSG objects and their PhysX representation.
 * It has to be initialized with a PhysicsWrapper to have an appropriate PhysicsScene and materials. 
 * The required osg parent has to be set at creation of every Object.
 * Later, Particle engines are supposed to be used inside here.
 */
class ObjectsContainer{
public:
    typedef std::pair<osg::MatrixTransform*, physx::PxRigidDynamic*> DrawableAndPhysXObject;
    /** Maximal number of particles before they begin to get destroyed. */
    static const physx::PxU32	kMaxParticleCount = 800;

    /** Explicit Constructor because Copying and Assignments are disabled. The wrapper is necessary to receive an initialited scene and initialized physics.*/
    explicit ObjectsContainer(std::shared_ptr<PhysicsWrapper> physics_wrapper);
    ~ObjectsContainer();

    /** Sets and rotates every OSG object according to its representation in PhysX. */
    void updateAllObjects();

    /** Creates a ball. */
    void makeStandardBall(osg::ref_ptr<osg::Group> parent, const physx::PxVec3& global_position, physx::PxReal radius, const physx::PxVec3& linear_velocity, const physx::PxVec3& angular_velocity);
    
    /** Creates a particle emitter */
    void makeParticleEmitter(osg::ref_ptr<osg::Group> parent, const physx::PxVec3& position);

    void createParticles(osg::ref_ptr<osg::Group> parent, int number_of_particles, const physx::PxVec3& position);


protected:
  
    std::shared_ptr<PhysicsWrapper>     m_physics_wrapper;
    std::list<DrawableAndPhysXObject>   m_objects;

    physx::PxParticleSystem*            m_particle_system;
    std::list<osg::MatrixTransform*>    m_particle_objects;

    void initializeParticles();
    void createParticleObject(osg::ref_ptr<osg::Group> parent, const physx::PxVec3& position);
private:
    DISALLOW_COPY_AND_ASSIGN(ObjectsContainer);
};
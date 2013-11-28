#pragma once

#include <osg/GraphicsContext> // ref_ptr
#include <memory> //shared_ptr


//Forward Declarations
class PhysicsWrapper;
class ObjectsContainer;
class ElemateHeightFieldTerrain;
class SoundManager;
namespace osg {         class Group;}


class World {
public:

    World();    
    ~World();


    osg::Group* root();

    /** Throws a standard osg ball into the game using the ObjectsContainer with correct physics.*/
	void makeStandardBall();
	void playSound();

    std::shared_ptr<PhysicsWrapper>             physics_wrapper;
	std::shared_ptr<ObjectsContainer>           objects_container;
	std::shared_ptr<ElemateHeightFieldTerrain>  terrain;

protected:
    
    osg::ref_ptr<osg::Group>            m_root;
};


#pragma once

#include <memory> //shared_ptr
#include <map>

#include <glm/glm.hpp>

class PhysicsWrapper;
class ObjectsContainer;
class GodNavigation;
class ElemateHeightFieldTerrain;
class SoundManager;

class World {
public:

    World();
    ~World();

    /** Throws a standard osg ball into the game using the ObjectsContainer with correct physics.*/
    void makeStandardBall(const glm::vec3& position);
    void createFountainSound();

    /** plays and pauses the background sound **/
    void toogleBackgroundSound(int id);

    void setNavigation(GodNavigation * navigation);
    void reloadShader();
    void setUniforms(long double globalTime);

    //osg::Program * programByName(const std::string & name) const;
    
    std::shared_ptr<PhysicsWrapper>             physics_wrapper;
    std::shared_ptr<ObjectsContainer>           objects_container;
    std::shared_ptr<ElemateHeightFieldTerrain>  terrain;
    std::shared_ptr<SoundManager>  soundManager;

protected:
    //std::map<std::string, osg::ref_ptr<osg::Program>>   m_programsByName;

    void setUpLighting();
    void initShader();
};

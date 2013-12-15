
#pragma once

#include <memory> //shared_ptr
#include <map>
#include <osg/ref_ptr>

class PhysicsWrapper;
class ObjectsContainer;
class GodNavigation;
class ElemateHeightFieldTerrain;
class SoundManager;
namespace osg {
    class Group;
    class Program;
    class MatrixTransform;
    class Matrixd;
}

class World {
public:

    World();
    ~World();

    osg::Group* root();
    osg::Matrixd getCameraTransform();

    /** Throws a standard osg ball into the game using the ObjectsContainer with correct physics.*/
    void makeStandardBall();
    void startFountainSound();
    void endFountainSound();
    void updateFountainPosition();

    /** plays and pauses the background sound **/
    void toogleBackgroundSound(int id);

    void setNavigation(GodNavigation * navigation);
    void reloadShader();
    void setUniforms(long double globalTime);

    osg::Program * programByName(const std::string & name) const;
    
    std::shared_ptr<PhysicsWrapper>             physics_wrapper;
    std::shared_ptr<ObjectsContainer>           objects_container;
    std::shared_ptr<ElemateHeightFieldTerrain>  terrain;
    std::shared_ptr<SoundManager>  soundManager;

protected:
    osg::ref_ptr<GodNavigation>                         m_navigation;
    osg::ref_ptr<osg::Group>                            m_root;
    osg::ref_ptr<osg::Group>                            m_particleGroup;
    osg::ref_ptr<osg::MatrixTransform>                  m_cameraDebugger;
    std::map<std::string, osg::ref_ptr<osg::Program>>   m_programsByName;

    void setUpLighting();
    void setUpCameraDebugger();
    void initShader();
};

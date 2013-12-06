
#pragma once

#include <memory> //shared_ptr
#include <map>
#include <osg/ref_ptr>

class PhysicsWrapper;
class ObjectsContainer;
class GodNavigation;
class ElemateHeightFieldTerrain;
class Hand;
namespace osg {
    class Group;
    class Program;
    class MatrixTransform;
}


class World {
public:

    World();
    ~World();

    osg::Group* root();

    /** Throws a standard osg ball into the game using the ObjectsContainer with correct physics.*/
    void makeStandardBall();

    void setNavigation(GodNavigation * navigation);
    void reloadShader();
    void setUniforms();
    

    std::shared_ptr<PhysicsWrapper>             physics_wrapper;
    std::shared_ptr<ObjectsContainer>           objects_container;
    std::shared_ptr<ElemateHeightFieldTerrain>  terrain;
    std::shared_ptr<Hand>                       hand;

protected:
    void setUpLighting();
    void setUpCameraDebugger();
    osg::ref_ptr<osg::Group> m_root;
    osg::ref_ptr<osg::Group> m_particleGroup;
    osg::ref_ptr<osg::MatrixTransform> m_cameraDebugger;
    void initShader();
    osg::Program * programByName(std::string name) const;
    std::map<std::string, osg::ref_ptr<osg::Program>> m_programsByName;
    osg::ref_ptr<GodNavigation> m_navigation;
};

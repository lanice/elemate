#pragma once

#include <memory> //shared_ptr
#include <map>
#include <osg/ref_ptr>

class PhysicsWrapper;
class ObjectsContainer;
class GodNavigation;
class ElemateHeightFieldTerrain;
namespace osg {
    class Group;
    class Program;
    class GraphicsContext;
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

    void setGraphicsContext(osg::GraphicsContext * context);
    

    std::shared_ptr<PhysicsWrapper>             physics_wrapper;
    std::shared_ptr<ObjectsContainer>           objects_container;
    std::shared_ptr<ElemateHeightFieldTerrain>  terrain;

protected:
    osg::ref_ptr<osg::GraphicsContext> m_graphicContext;

    osg::ref_ptr<osg::Group> m_root;
    osg::ref_ptr<osg::Group> m_particleGroup;
    osg::ref_ptr<GodNavigation> m_navigation;

    void setUpLighting();
    void initShader();
    osg::Program * programByName(std::string name) const;
    std::map<std::string, osg::ref_ptr<osg::Program>> m_programsByName;
};


#include "godmanipulator.h"

#include <cassert>

#include <osg/MatrixTransform>
#include <osg/Camera>
#include <osg/Texture2D>
#include <osg/Image>

#include "world.h"
#include "hand.h"


GodManipulator::GodManipulator()
   : inherited(),
     m_world(nullptr),
     m_camera(nullptr),
     m_depth(nullptr),
     m_hand(new Hand())
{
}


GodManipulator::GodManipulator( const GodManipulator& gm, const osg::CopyOp& copyOp )
   : Object(gm, copyOp),
     inherited( gm, copyOp ),
     m_world( gm.m_world ),
     m_camera( gm.m_camera ),
     m_depth( gm.m_depth ),
     m_hand( gm.m_hand )
{
}


GodManipulator::~GodManipulator()
{
    delete m_hand;
}


void GodManipulator::setWorld( std::shared_ptr<World> world )
{
    m_world = world;
    m_world->root()->addChild( m_hand->transform() );
}


void GodManipulator::setCamera( osg::Camera * camera )
{
    m_camera = camera;
}


void GodManipulator::setDepthBuffer( osg::Texture2D * buffer )
{
    m_depth = buffer;
}


bool GodManipulator::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
{
    if( ea.getHandled() )
        return false;

    switch( ea.getEventType() )
    {
        case osgGA::GUIEventAdapter::MOVE:
            return handleMouseMove( ea, us );

        case osgGA::GUIEventAdapter::DRAG:
            return handleMouseDrag( ea, us );

        case osgGA::GUIEventAdapter::PUSH:
            return handleMousePush( ea, us );

        case osgGA::GUIEventAdapter::RELEASE:
            return handleMouseRelease( ea, us );

        case osgGA::GUIEventAdapter::KEYDOWN:
            return handleKeyDown( ea, us );

        case osgGA::GUIEventAdapter::KEYUP:
            return handleKeyUp( ea, us );

        case osgGA::GUIEventAdapter::SCROLL:
            return handleMouseWheel( ea, us );

        default:
            return false;
    }
}


bool GodManipulator::handleMouseMove( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*us*/ )
{
    assert(m_depth);
    // osg::Vec4 color = m_depth->getImage()->getColor( osg::Vec2d(ea.getX(), ea.getY()) );
    osg::Image * image = m_depth->getImage();
    assert(image);

    osg::Matrixd matrix = (m_camera->getProjectionMatrix() * m_world->getCameraTransform() * m_camera->getViewMatrix());
    osg::Matrixd::inverse(matrix);

    osg::Vec3 pos = osg::Vec3( (ea.getX() / ea.getWindowWidth() - 0.5) * 2., (ea.getY() / ea.getWindowHeight() - 0.5) * 2., -1. ) * matrix;
    m_hand->transform()->setMatrix( m_hand->defaultTransform() * osg::Matrixd::translate( pos ) );
    return true;
}


bool GodManipulator::handleMouseDrag( const osgGA::GUIEventAdapter& /*ea*/, osgGA::GUIActionAdapter& /*us*/ )
{
    return false;
}


bool GodManipulator::handleMousePush( const osgGA::GUIEventAdapter& /*ea*/, osgGA::GUIActionAdapter& /*us*/ )
{
    return false;
}


bool GodManipulator::handleMouseRelease( const osgGA::GUIEventAdapter& /*ea*/, osgGA::GUIActionAdapter& /*us*/ )
{
    return false;
}


bool GodManipulator::handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*us*/ )
{
    switch (ea.getUnmodifiedKey()){
        case osgGA::GUIEventAdapter::KEY_P:
        {
            m_world->toogleBackgroundSound(0);
            return true;
        }
        case osgGA::GUIEventAdapter::KEY_O:
        {
            m_world->toogleBackgroundSound(1);
            return true;
        }
        case osgGA::GUIEventAdapter::KEY_F:
        {
            m_world->makeStandardBall();
            if (!isFountainOn){
                m_world->startFountainSound();
                isFountainOn = true;
            }
            m_world->updateFountainPosition();
            return true;
        }
        case osgGA::GUIEventAdapter::KEY_F5:
        {
            m_world->reloadShader();
            return true;
        }
    }
    return false;
}


bool GodManipulator::handleKeyUp( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*us*/ )
{
    switch (ea.getUnmodifiedKey()){
    case osgGA::GUIEventAdapter::KEY_F:
    {
        m_world->endFountainSound();
        isFountainOn = false;
        return true;
    }
    }
    return false;
}


bool GodManipulator::handleMouseWheel( const osgGA::GUIEventAdapter& /*ea*/, osgGA::GUIActionAdapter& /*us*/ )
{
    return false;
}

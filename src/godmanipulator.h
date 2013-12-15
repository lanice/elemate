
#pragma once


#include <osgGA/GUIEventHandler>

#include <memory> //shared_ptr


class World;

/** The EventHandler for game content/logic specific events.
 *  To process an incoming event modify the according handle* class.
 */
class GodManipulator : public osgGA::GUIEventHandler
{
        typedef osgGA::GUIEventHandler inherited;

    public:

        GodManipulator();
        GodManipulator( const GodManipulator& gm, const osg::CopyOp& copyOp = osg::CopyOp::SHALLOW_COPY );


        void setWorld( std::shared_ptr<World> world ) { m_world = world; };


        /** Handles events. Returns true if handled, false otherwise.*/
        virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
        /** Handles GUIEventAdapter::MOVE event.*/
        virtual bool handleMouseMove( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
        /** Handles GUIEventAdapter::DRAG event.*/
        virtual bool handleMouseDrag( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
        /** Handles GUIEventAdapter::PUSH event.*/
        virtual bool handleMousePush( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
        /** Handles GUIEventAdapter::RELEASE event.*/
        virtual bool handleMouseRelease( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
        /** Handles GUIEventAdapter::KEYDOWN event.*/
        virtual bool handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
        /** Handles GUIEventAdapter::KEYUP event.*/
        virtual bool handleKeyUp( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
        /** Handles GUIEventAdapter::SCROLL event.*/
        virtual bool handleMouseWheel( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );


    protected:

        std::shared_ptr<World> m_world;
        bool isFountainOn = false;
};

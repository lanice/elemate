
#pragma once

#include "world.h"

#include <osgGA/GUIEventHandler>


class GodManipulator : public osgGA::GUIEventHandler
{
        typedef osgGA::GUIEventHandler inherited;

    public:

        GodManipulator();
        GodManipulator( const GodManipulator& gm, const osg::CopyOp& copyOp = osg::CopyOp::SHALLOW_COPY );


        void setWorld( std::shared_ptr<World> world ) { m_world = world; };


        /** Handles events. Returns true if handled, false otherwise.*/
        virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
        /** Handles GUIEventAdapter::KEYDOWN event.*/
        virtual bool handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );


    protected:

        std::shared_ptr<World> m_world;
};


#pragma once

#include <osgGA/GUIEventHandler>



class GodEventHandler : public osgGA::GUIEventHandler
{
        typedef osgGA::GUIEventHandler inherited;

    public:

        GodEventHandler();
        GodEventHandler( const GodEventHandler& geh, const osg::CopyOp& copyOp = osg::CopyOp::SHALLOW_COPY );

        /** Handles events. Returns true if handled, false otherwise.*/
        virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
};

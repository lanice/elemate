
#pragma once

#include <osgGA/GUIEventHandler>



class GodManipulator : public osgGA::GUIEventHandler
{
        typedef osgGA::GUIEventHandler inherited;

    public:

        GodManipulator();
        GodManipulator( const GodManipulator& geh, const osg::CopyOp& copyOp = osg::CopyOp::SHALLOW_COPY );

        /** Handles events. Returns true if handled, false otherwise.*/
        virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
};

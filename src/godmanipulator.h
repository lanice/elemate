
#pragma once

#include <osgGA/StandardManipulator>



class GodManipulator : public osgGA::StandardManipulator
{
        typedef osgGA::StandardManipulator inherited;

    public:

        GodManipulator( int flags = DEFAULT_SETTINGS );
        GodManipulator( const GodManipulator& fpm, const osg::CopyOp& copyOp = osg::CopyOp::SHALLOW_COPY );

        /** Set the position of the manipulator using a 4x4 matrix.*/
        virtual void setByMatrix( const osg::Matrixd& matrix );
        /** Set the position of the manipulator using a 4x4 matrix.*/
        virtual void setByInverseMatrix( const osg::Matrixd& matrix );
        /** Get the position of the manipulator as 4x4 matrix.*/
        virtual osg::Matrixd getMatrix() const;
        /** Get the position of the manipulator as a inverse matrix of the manipulator, typically used as a model view matrix.*/
        virtual osg::Matrixd getInverseMatrix() const;

        /** Set manipulator by eye position and eye orientation.*/
        virtual void setTransformation( const osg::Vec3d& eye, const osg::Quat& rotation );
        /** Get manipulator's eye position and eye orientation.*/
        virtual void setTransformation( const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up );
        /** Set manipulator by eye position, center of rotation, and up vector.*/
        virtual void getTransformation( osg::Vec3d& eye, osg::Quat& rotation ) const;
        /** Get manipulator's focal center, eye position, and up vector.*/
        virtual void getTransformation( osg::Vec3d& eye, osg::Vec3d& center, osg::Vec3d& up ) const;

    protected:

        /** Handles GUIEventAdapter::KEYDOWN event.*/
        virtual bool handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );

        /** Move camera forward by distance parameter.*/
        virtual void moveForward( const double distance );
        /** Move camera forward by distance parameter.*/
        virtual void moveForward( const osg::Quat& rotation, const double distance );
        /** Move camera right by distance parameter.*/
        virtual void moveRight( const double distance );
        /** Move camera up by distance parameter.*/
        virtual void moveUp( const double distance );

        osg::Vec3d _eye;
        osg::Quat  _rotation;
};

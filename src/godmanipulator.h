
#pragma once

#include <osgGA/StandardManipulator>



class GodManipulator : public osgGA::StandardManipulator
{
        typedef osgGA::StandardManipulator inherited;

    public:

        GodManipulator( int flags = DEFAULT_SETTINGS );
        GodManipulator( const GodManipulator& fpm, const osg::CopyOp& copyOp = osg::CopyOp::SHALLOW_COPY );

        virtual void setByMatrix( const osg::Matrixd& matrix );
        virtual void setByInverseMatrix( const osg::Matrixd& matrix );
        virtual osg::Matrixd getMatrix() const;
        virtual osg::Matrixd getInverseMatrix() const;

        virtual void setTransformation( const osg::Vec3d& eye, const osg::Quat& rotation );
        virtual void setTransformation( const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up );
        virtual void getTransformation( osg::Vec3d& eye, osg::Quat& rotation ) const;
        virtual void getTransformation( osg::Vec3d& eye, osg::Vec3d& center, osg::Vec3d& up ) const;

    protected:

        virtual bool handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );

        virtual void moveForward( const double distance );
        virtual void moveForward( const osg::Quat& rotation, const double distance );
        virtual void moveRight( const double distance );
        virtual void moveUp( const double distance );

        osg::Vec3d _eye;
        osg::Quat  _rotation;
};


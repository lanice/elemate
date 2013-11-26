
#pragma once

#include <osgGA/StandardManipulator>



class GodManipulator : public osgGA::StandardManipulator
{
        typedef osgGA::StandardManipulator inherited;

    public:

        GodManipulator( int flags = DEFAULT_SETTINGS );
        GodManipulator( const GodManipulator& fpm, const osg::CopyOp& copyOp = osg::CopyOp::SHALLOW_COPY );

        META_Object(osgGA, GodManipulator);

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

        /** Set distance per frame the manipulator should move.*/
        virtual void setVelocity( const double& velocity );
        /** Get distance per frame the manipulator moves.*/
        virtual double getVelocity() const;

    protected:

        /** Handles GUIEventAdapter::KEYDOWN event.*/
        virtual bool handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
        /** Handles GUIEventAdapter::KEYUP event.*/
        virtual bool handleKeyUp( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );

        /** Set requestContinuousUpdate( bool ) to false if no movement key is pressed.*/
        virtual void disableContinuousUpdateIfNecessary( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );

        /** Make movement step of manipulator. Returns true if any movement was made.*/
        virtual bool performMovement();
        /** Move the camera in direction movementDirection by distance parameter.*/
        virtual bool performMovement( const osg::Vec3d& movementDirection, const double distance );
        /** Rotate the camera by yaw parameter.*/
        virtual bool performRotationYaw( const double yaw );

        /** Calculate movement direction when pressing W key and adding it to movementDirection vector.*/
        virtual void calculateMovementDirectionKeyW( osg::Vec3d& movementDirection );
        /** Calculate movement direction when pressing S key and adding it to movementDirection vector.*/
        virtual void calculateMovementDirectionKeyS( osg::Vec3d& movementDirection );
        /** Calculate movement direction when pressing A key and adding it to movementDirection vector.*/
        virtual void calculateMovementDirectionKeyA( osg::Vec3d& movementDirection );
        /** Calculate movement direction when pressing D key and adding it to movementDirection vector.*/
        virtual void calculateMovementDirectionKeyD( osg::Vec3d& movementDirection );
        /** Calculate movement direction when pressing Q key and adding it to movementDirection vector.*/
        virtual void calculateMovementDirectionKeyQ( osg::Vec3d& movementDirection );
        /** Calculate movement direction when pressing E key and adding it to movementDirection vector.*/
        virtual void calculateMovementDirectionKeyE( osg::Vec3d& movementDirection );
        
        /** Move camera forward by distance parameter.*/
        virtual void moveForward( const double distance );
        /** Move camera right by distance parameter.*/
        virtual void moveRight( const double distance );
        /** Move camera up by distance parameter.*/
        virtual void moveUp( const double distance );


        osg::Vec3d _eye;
        osg::Quat  _rotation;
        double _velocity;

        bool _keyPressedW;
        bool _keyPressedS;
        bool _keyPressedA;
        bool _keyPressedD;
        bool _keyPressedQ;
        bool _keyPressedE;
};

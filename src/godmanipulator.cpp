
#include "godmanipulator.h"



/// Constructor.
GodManipulator::GodManipulator( int flags )
   : inherited( flags )
{
    // setAcceleration( 1.0, true );
    // setMaxVelocity( 0.25, true );
    // setWheelMovement( 0.05, true );
    if( _flags & SET_CENTER_ON_WHEEL_FORWARD_MOVEMENT )
        setAnimationTime( 0.2 );
}


/// Constructor.
GodManipulator::GodManipulator( const GodManipulator& gpm, const osg::CopyOp& copyOp )
   : Object(gpm, copyOp),
     inherited( gpm, copyOp ),
     _eye( gpm._eye ),
     _rotation( gpm._rotation )//,
     // _velocity( gpm._velocity ),
     // _acceleration( gpm._acceleration ),
     // _maxVelocity( gpm._maxVelocity ),
     // _wheelMovement( gpm._wheelMovement )
{
}


/** Set the position of the manipulator using a 4x4 matrix.*/
void GodManipulator::setByMatrix( const osg::Matrixd& matrix )
{
   // set variables
   _eye = matrix.getTrans();
   _rotation = matrix.getRotate();

   // fix current rotation
   if( getVerticalAxisFixed() )
      fixVerticalAxis( _eye, _rotation, true );
}


/** Set the position of the manipulator using a 4x4 matrix.*/
void GodManipulator::setByInverseMatrix( const osg::Matrixd& matrix )
{
   setByMatrix( osg::Matrixd::inverse( matrix ) );
}


/** Get the position of the manipulator as 4x4 matrix.*/
osg::Matrixd GodManipulator::getMatrix() const
{
   return osg::Matrixd::rotate( _rotation ) * osg::Matrixd::translate( _eye );
}


/** Get the position of the manipulator as a inverse matrix of the manipulator,
    typically used as a model view matrix.*/
osg::Matrixd GodManipulator::getInverseMatrix() const
{
   return osg::Matrixd::translate( -_eye ) * osg::Matrixd::rotate( _rotation.inverse() );
}


/** Sets manipulator by eye position and eye orientation.*/
void GodManipulator::setTransformation( const osg::Vec3d& eye, const osg::Quat& rotation )
{
   // set variables
   _eye = eye;
   _rotation = rotation;

   // fix current rotation
   if( getVerticalAxisFixed() )
      fixVerticalAxis( _eye, _rotation, true );
}


/** Gets manipulator's eye position and eye orientation.*/
void GodManipulator::getTransformation( osg::Vec3d& eye, osg::Quat& rotation ) const
{
   eye = _eye;
   rotation = _rotation;
}


/** Sets manipulator by eye position, center of rotation, and up vector.*/
void GodManipulator::setTransformation( const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up )
{
   // set variables
   osg::Matrixd m( osg::Matrixd::lookAt( eye, center, up ) );
   _eye = eye;
   _rotation = m.getRotate().inverse();

   // fix current rotation
   if( getVerticalAxisFixed() )
      fixVerticalAxis( _eye, _rotation, true );
}


/** Gets manipulator's focal center, eye position, and up vector.*/
void GodManipulator::getTransformation( osg::Vec3d& eye, osg::Vec3d& center, osg::Vec3d& up ) const
{
   center = _eye + _rotation * osg::Vec3d( 0.,0.,-1. );
   eye = _eye;
   up = _rotation * osg::Vec3d( 0.,1.,0. );
}


/// Handles GUIEventAdapter::KEYDOWN event.
bool GodManipulator::handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
{
    switch( ea.getKey() )
    {
        case osgGA::GUIEventAdapter::KEY_Space:
            flushMouseEventStack();
            _thrown = false;
            home(ea,us);
            return true;

        case osgGA::GUIEventAdapter::KEY_W:
            // move forward
            moveForward( 1.0 );
            us.requestRedraw();
            us.requestContinuousUpdate( true );
            return true;

        case osgGA::GUIEventAdapter::KEY_S:
            // move backward
            moveForward( -1.0 );
            us.requestRedraw();
            us.requestContinuousUpdate( true );
            return true;

        case osgGA::GUIEventAdapter::KEY_A:
            // move left
            moveRight( -1.0 );
            us.requestRedraw();
            us.requestContinuousUpdate( true );
            return true;

        case osgGA::GUIEventAdapter::KEY_D:
            // move right
            moveRight( 1.0 );
            us.requestRedraw();
            us.requestContinuousUpdate( true );
            return true;

        default:
            return false;
    }

    return false;
}


/// Move camera forward by distance parameter.
void GodManipulator::moveForward( const double distance )
{
   moveForward( _rotation, distance );
}


/// Move camera forward by distance parameter.
void GodManipulator::moveForward( const osg::Quat& rotation, const double distance )
{
   _eye += rotation * osg::Vec3d( 0., 0., -distance );
}


/// Move camera right by distance parameter.
void GodManipulator::moveRight( const double distance )
{
   _eye += _rotation * osg::Vec3d( distance, 0., 0. );
}


/// Move camera up by distance parameter.
void GodManipulator::moveUp( const double distance )
{
   _eye += _rotation * osg::Vec3d( 0., distance, 0. );
}

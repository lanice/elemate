
#include "godmanipulator.h"



GodManipulator::GodManipulator( int flags )
   : inherited( flags )
{
    // setAcceleration( 1.0, true );
    // setMaxVelocity( 0.25, true );
    // setWheelMovement( 0.05, true );
    if( _flags & SET_CENTER_ON_WHEEL_FORWARD_MOVEMENT )
        setAnimationTime( 0.2 );
}


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


void GodManipulator::setByMatrix( const osg::Matrixd& matrix )
{
   // set variables
   _eye = matrix.getTrans();
   _rotation = matrix.getRotate();

   // fix current rotation
   if( getVerticalAxisFixed() )
      fixVerticalAxis( _eye, _rotation, true );
}


void GodManipulator::setByInverseMatrix( const osg::Matrixd& matrix )
{
   setByMatrix( osg::Matrixd::inverse( matrix ) );
}


osg::Matrixd GodManipulator::getMatrix() const
{
   return osg::Matrixd::rotate( _rotation ) * osg::Matrixd::translate( _eye );
}


osg::Matrixd GodManipulator::getInverseMatrix() const
{
   return osg::Matrixd::translate( -_eye ) * osg::Matrixd::rotate( _rotation.inverse() );
}


void GodManipulator::setTransformation( const osg::Vec3d& eye, const osg::Quat& rotation )
{
   // set variables
   _eye = eye;
   _rotation = rotation;

   // fix current rotation
   if( getVerticalAxisFixed() )
      fixVerticalAxis( _eye, _rotation, true );
}


void GodManipulator::getTransformation( osg::Vec3d& eye, osg::Quat& rotation ) const
{
   eye = _eye;
   rotation = _rotation;
}


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


void GodManipulator::getTransformation( osg::Vec3d& eye, osg::Vec3d& center, osg::Vec3d& up ) const
{
   center = _eye + _rotation * osg::Vec3d( 0.,0.,-1. );
   eye = _eye;
   up = _rotation * osg::Vec3d( 0.,1.,0. );
}


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
            us.requestContinuousUpdate( true );
            moveForward( 1.0 );
            us.requestRedraw();
            return true;

        case osgGA::GUIEventAdapter::KEY_S:
            // move backward
            us.requestContinuousUpdate( true );
            moveForward( -1.0 );
            us.requestRedraw();
            return true;

        case osgGA::GUIEventAdapter::KEY_A:
            // move left
            us.requestContinuousUpdate( true );
            moveRight( -1.0 );
            us.requestRedraw();
            return true;

        case osgGA::GUIEventAdapter::KEY_D:
            // move right
            us.requestContinuousUpdate( true );
            moveRight( 1.0 );
            us.requestRedraw();
            return true;

        default:
            return false;
    }

    return false;
}


void GodManipulator::moveForward( const double distance )
{
   moveForward( _rotation, distance );
}


void GodManipulator::moveForward( const osg::Quat& rotation, const double distance )
{
   _eye += rotation * osg::Vec3d( 0., 0., -distance );
}


void GodManipulator::moveRight( const double distance )
{
   _eye += _rotation * osg::Vec3d( distance, 0., 0. );
}


void GodManipulator::moveUp( const double distance )
{
   _eye += _rotation * osg::Vec3d( 0., distance, 0. );
}

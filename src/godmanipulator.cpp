
#include "godmanipulator.h"


static const double c_velocityNormal = 1.0;


GodManipulator::GodManipulator( int flags )
   : inherited( flags ),
     _keyPressedW( false ),
     _keyPressedS( false ),
     _keyPressedA( false ),
     _keyPressedD( false )
{
    // setAcceleration( 1.0, true );
    // setMaxVelocity( 0.25, true );
    // setWheelMovement( 0.05, true );
    setVelocity( c_velocityNormal );
    if( _flags & SET_CENTER_ON_WHEEL_FORWARD_MOVEMENT )
        setAnimationTime( 0.2 );
}


GodManipulator::GodManipulator( const GodManipulator& gpm, const osg::CopyOp& copyOp )
   : Object(gpm, copyOp),
     inherited( gpm, copyOp ),
     _eye( gpm._eye ),
     _rotation( gpm._rotation ),
     _velocity( gpm._velocity ),
     _keyPressedW( gpm._keyPressedW ),
     _keyPressedS( gpm._keyPressedS ),
     _keyPressedA( gpm._keyPressedA ),
     _keyPressedD( gpm._keyPressedD )//,
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


void GodManipulator::setVelocity( const double& velocity )
{
   _velocity = velocity;
}


double GodManipulator::getVelocity() const
{
   return _velocity;
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
            us.requestRedraw();
            us.requestContinuousUpdate( true );
            _keyPressedW = true;
            _thrown = true;
            return true;

        case osgGA::GUIEventAdapter::KEY_S:
            us.requestRedraw();
            us.requestContinuousUpdate( true );
            _keyPressedS = true;
            _thrown = true;
            return true;

        case osgGA::GUIEventAdapter::KEY_A:
            us.requestRedraw();
            us.requestContinuousUpdate( true );
            _keyPressedA = true;
            _thrown = true;
            return true;

        case osgGA::GUIEventAdapter::KEY_D:
            us.requestRedraw();
            us.requestContinuousUpdate( true );
            _keyPressedD = true;
            _thrown = true;
            return true;

        default:
            return false;
    }

    return false;
}


bool GodManipulator::handleKeyUp( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
{
    switch( ea.getKey() )
    {
        case osgGA::GUIEventAdapter::KEY_W:
            _keyPressedW = false;
            disableContinuousUpdateIfNecessary( ea, us );
            return true;

        case osgGA::GUIEventAdapter::KEY_S:
            _keyPressedS = false;
            disableContinuousUpdateIfNecessary( ea, us );
            return true;

        case osgGA::GUIEventAdapter::KEY_A:
            _keyPressedA = false;
            disableContinuousUpdateIfNecessary( ea, us );
            return true;

        case osgGA::GUIEventAdapter::KEY_D:
            _keyPressedD = false;
            disableContinuousUpdateIfNecessary( ea, us );
            return true;

        default:
            return false;
    }

    return false;
}


void GodManipulator::disableContinuousUpdateIfNecessary( const osgGA::GUIEventAdapter& /*ea*/, osgGA::GUIActionAdapter& us )
{
    if ( !_keyPressedW && !_keyPressedS && !_keyPressedA && !_keyPressedD )
    {
        _thrown = false;
        us.requestContinuousUpdate( false );
    }
}


bool GodManipulator::performMovement()
{
    /*// return if less then two events have been added
    if( _ga_t0.get() == NULL || _ga_t1.get() == NULL )
        return false;

    // get delta time
    double eventTimeDelta = _ga_t0->getTime() - _ga_t1->getTime();
    if( eventTimeDelta < 0. )
    {
        OSG_WARN << "Manipulator warning: eventTimeDelta = " << eventTimeDelta << std::endl;
        eventTimeDelta = 0.;
    }

    // get deltaX and deltaY
    float dx = _ga_t0->getXnormalized() - _ga_t1->getXnormalized();
    float dy = _ga_t0->getYnormalized() - _ga_t1->getYnormalized();

    // return if there is no movement.
    if( dx == 0. && dy == 0. )
        return false;*/

    bool moved = false;
    
    // call appropriate methods
    if ( _keyPressedW )
        moved = performMovementKeyW( c_velocityNormal );
    if ( _keyPressedS )
        moved = performMovementKeyS( c_velocityNormal );
    if ( _keyPressedA )
        moved = performMovementKeyA( c_velocityNormal );
    if ( _keyPressedD )
        moved = performMovementKeyD( c_velocityNormal );

    return moved;
}


bool GodManipulator::performMovementKeyW( const double distance )
{
    moveForward( distance );
    return true;
}


bool GodManipulator::performMovementKeyS( const double distance )
{
    moveForward( -distance );
    return true;
}


bool GodManipulator::performMovementKeyA( const double distance )
{
    moveRight( -distance );
    return true;
}


bool GodManipulator::performMovementKeyD( const double distance )
{
    moveRight( distance );
    return true;
}


void GodManipulator::moveForward( const double distance )
{
   _eye += _rotation * osg::Vec3d( 0., 0., -distance );
}


void GodManipulator::moveRight( const double distance )
{
   _eye += _rotation * osg::Vec3d( distance, 0., 0. );
}


void GodManipulator::moveUp( const double distance )
{
   _eye += _rotation * osg::Vec3d( 0., distance, 0. );
}

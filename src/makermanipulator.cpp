
#include "makermanipulator.h"


static const double c_velocityNormal = 0.2;


MakerManipulator::MakerManipulator( int flags )
   : inherited( flags ),
     _keyPressedW( false ),
     _keyPressedS( false ),
     _keyPressedA( false ),
     _keyPressedD( false ),
     _keyPressedQ( false ),
     _keyPressedE( false )
{
    // setAcceleration( 1.0, true );
    // setMaxVelocity( 0.25, true );
    // setWheelMovement( 0.05, true );
    setVelocity( c_velocityNormal );
    if( _flags & SET_CENTER_ON_WHEEL_FORWARD_MOVEMENT )
        setAnimationTime( 0.2 );
}


MakerManipulator::MakerManipulator( const MakerManipulator& gpm, const osg::CopyOp& copyOp )
   : Object(gpm, copyOp),
     inherited( gpm, copyOp ),
     _eye( gpm._eye ),
     _rotation( gpm._rotation ),
     _velocity( gpm._velocity ),
     _keyPressedW( gpm._keyPressedW ),
     _keyPressedS( gpm._keyPressedS ),
     _keyPressedA( gpm._keyPressedA ),
     _keyPressedD( gpm._keyPressedD ),
     _keyPressedQ( gpm._keyPressedA ),
     _keyPressedE( gpm._keyPressedD )//,
     // _acceleration( gpm._acceleration ),
     // _maxVelocity( gpm._maxVelocity ),
     // _wheelMovement( gpm._wheelMovement )
{
}


void MakerManipulator::setByMatrix( const osg::Matrixd& matrix )
{
   // set variables
   _eye = matrix.getTrans();
   _rotation = matrix.getRotate();

   // fix current rotation
   if( getVerticalAxisFixed() )
      fixVerticalAxis( _eye, _rotation, true );
}


void MakerManipulator::setByInverseMatrix( const osg::Matrixd& matrix )
{
   setByMatrix( osg::Matrixd::inverse( matrix ) );
}


osg::Matrixd MakerManipulator::getMatrix() const
{
   return osg::Matrixd::rotate( _rotation ) * osg::Matrixd::translate( _eye );
}


osg::Matrixd MakerManipulator::getInverseMatrix() const
{
   return osg::Matrixd::translate( -_eye ) * osg::Matrixd::rotate( _rotation.inverse() );
}


void MakerManipulator::setTransformation( const osg::Vec3d& eye, const osg::Quat& rotation )
{
   // set variables
   _eye = eye;
   _rotation = rotation;

   // fix current rotation
   if( getVerticalAxisFixed() )
      fixVerticalAxis( _eye, _rotation, true );
}


void MakerManipulator::getTransformation( osg::Vec3d& eye, osg::Quat& rotation ) const
{
   eye = _eye;
   rotation = _rotation;
}


void MakerManipulator::setTransformation( const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up )
{
   // set variables
   osg::Matrixd m( osg::Matrixd::lookAt( eye, center, up ) );
   _eye = eye;
   _rotation = m.getRotate().inverse();

   // fix current rotation
   if( getVerticalAxisFixed() )
      fixVerticalAxis( _eye, _rotation, true );
}


void MakerManipulator::getTransformation( osg::Vec3d& eye, osg::Vec3d& center, osg::Vec3d& up ) const
{
   center = _eye + _rotation * osg::Vec3d( 0.,0.,-1. );
   eye = _eye;
   up = _rotation * osg::Vec3d( 0.,1.,0. );
}


void MakerManipulator::setVelocity( const double& velocity )
{
   _velocity = velocity;
}


double MakerManipulator::getVelocity() const
{
   return _velocity;
}


bool MakerManipulator::handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
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

        case osgGA::GUIEventAdapter::KEY_Q:
            us.requestRedraw();
            us.requestContinuousUpdate( true );
            _keyPressedQ = true;
            _thrown = true;
            return true;

        case osgGA::GUIEventAdapter::KEY_E:
            us.requestRedraw();
            us.requestContinuousUpdate( true );
            _keyPressedE = true;
            _thrown = true;
            return true;

        default:
            return false;
    }

    return false;
}


bool MakerManipulator::handleKeyUp( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
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

        case osgGA::GUIEventAdapter::KEY_Q:
            _keyPressedQ = false;
            disableContinuousUpdateIfNecessary( ea, us );
            return true;

        case osgGA::GUIEventAdapter::KEY_E:
            _keyPressedE = false;
            disableContinuousUpdateIfNecessary( ea, us );
            return true;

        default:
            return false;
    }

    return false;
}


void MakerManipulator::disableContinuousUpdateIfNecessary( const osgGA::GUIEventAdapter& /*ea*/, osgGA::GUIActionAdapter& us )
{
    if ( !_keyPressedW && !_keyPressedS && !_keyPressedA && !_keyPressedD && !_keyPressedQ && !_keyPressedE )
    {
        _thrown = false;
        us.requestContinuousUpdate( false );
    }
}


// This method is still under construction!
bool MakerManipulator::performMovement()
{
    bool moved = false;
    double movementSpeed = c_velocityNormal;
    double yaw = 0.;
    double distanceToLookAtFactor = 3.;
    
    osg::Vec3d movementDirection = osg::Vec3d( 0., 0., 0. );
    osg::Vec3d rotateDirection = osg::Vec3d( 0., 0., 0. );

    // call appropriate methods
    if ( _keyPressedW )
        calculateMovementDirectionKeyW( movementDirection );
    if ( _keyPressedS )
        calculateMovementDirectionKeyS( movementDirection );
    if ( _keyPressedA )
        calculateMovementDirectionKeyA( movementDirection );
    if ( _keyPressedD )
        calculateMovementDirectionKeyD( movementDirection );
    if ( _keyPressedQ ) {
        calculateMovementDirectionKeyQ( rotateDirection );
        yaw += 0.1;
    }
    if ( _keyPressedE ) {
        calculateMovementDirectionKeyE( rotateDirection );
        yaw -= 0.1;
    }


    if ( movementDirection.length() != 0 ) {
        performMovement( movementDirection, movementSpeed );
        moved = true;
    }

    if ( yaw != 0 ) {
        performMovement( rotateDirection, movementSpeed*distanceToLookAtFactor );
        performRotationYaw( yaw/distanceToLookAtFactor );
        moved = true;
    }

    return moved;
}


bool MakerManipulator::performMovement( const osg::Vec3d& movementDirection, const double distance )
{
    osg::Vec3d direction = movementDirection;
    direction.normalize();
    _eye += (direction * distance);
    return true;
}


bool MakerManipulator::performRotationYaw( const double yaw )
{
    // world up vector to rotate with fixed Up vector
    osg::CoordinateFrame coordinateFrame = getCoordinateFrame( _eye );
    osg::Vec3d localUp = getUpVector( coordinateFrame );

    rotateYawPitch( _rotation, yaw, 0., localUp );
    return true;
}


void MakerManipulator::calculateMovementDirectionKeyW( osg::Vec3d& movementDirection )
{
    osg::Vec3d lookAtFront = _rotation * osg::Vec3d( 0., 0., -1. );

    movementDirection += osg::Vec3d( lookAtFront.x(), 0., lookAtFront.z() );
}


void MakerManipulator::calculateMovementDirectionKeyS( osg::Vec3d& movementDirection )
{
    osg::Vec3d lookAtBack = _rotation * osg::Vec3d( 0., 0., 1. );

    movementDirection += osg::Vec3d( lookAtBack.x(), 0., lookAtBack.z() );
}


void MakerManipulator::calculateMovementDirectionKeyA( osg::Vec3d& movementDirection )
{
    osg::Vec3d lookAtLeft = _rotation * osg::Vec3d( -1., 0., 0. );

    movementDirection += osg::Vec3d( lookAtLeft.x(), 0., lookAtLeft.z() );
}


void MakerManipulator::calculateMovementDirectionKeyD( osg::Vec3d& movementDirection )
{
    osg::Vec3d lookAtRight = _rotation * osg::Vec3d( 1., 0., 0. );

    movementDirection += osg::Vec3d( lookAtRight.x(), 0., lookAtRight.z() );
}


void MakerManipulator::calculateMovementDirectionKeyQ( osg::Vec3d& movementDirection )
{
    osg::Vec3d lookAtLeft = _rotation * osg::Vec3d( -1., 0., 0. );

    movementDirection += osg::Vec3d( lookAtLeft.x(), 0., lookAtLeft.z() );
}


void MakerManipulator::calculateMovementDirectionKeyE( osg::Vec3d& movementDirection )
{
    osg::Vec3d lookAtRight = _rotation * osg::Vec3d( 1., 0., 0. );

    movementDirection += osg::Vec3d( lookAtRight.x(), 0., lookAtRight.z() );
}


void MakerManipulator::moveForward( const double distance )
{
   _eye += _rotation * osg::Vec3d( 0., 0., -distance );
}


void MakerManipulator::moveRight( const double distance )
{
   _eye += _rotation * osg::Vec3d( distance, 0., 0. );
}


void MakerManipulator::moveUp( const double distance )
{
   _eye += _rotation * osg::Vec3d( 0., distance, 0. );
}

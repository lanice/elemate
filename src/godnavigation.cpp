
#include "godnavigation.h"

#include "world.h"
#include "terrain/elemateterrain.h"
#include "soundmanager.h"


static const double c_velocityDefault = 0.2;
static const double c_distanceEyeCenterDefault = 20.;


GodNavigation::GodNavigation( int flags )
   : inherited( flags ),
     _velocity( c_velocityDefault ),
     _distanceEyeCenter( c_distanceEyeCenterDefault ),
     _keyPressedW( false ),
     _keyPressedS( false ),
     _keyPressedA( false ),
     _keyPressedD( false ),
     _keyPressedQ( false ),
     _keyPressedE( false ),
     _keyPressedShift_L( false ),
     _slerping( false ),
     m_world(nullptr)
{
    if( _flags & SET_CENTER_ON_WHEEL_FORWARD_MOVEMENT )
        setAnimationTime( 0.2 );
}


GodNavigation::GodNavigation( const GodNavigation& gn, const osg::CopyOp& copyOp )
   : Object(gn, copyOp),
     inherited( gn, copyOp ),
     _center( gn._center ),
     _rotation( gn._rotation ),
     _startRotation( gn._startRotation ),
     _stopRotation( gn._stopRotation ),
     _startDistanceEyeCenter( gn._startDistanceEyeCenter ),
     _stopDistanceEyeCenter( gn._stopDistanceEyeCenter ),
     _stopTime( gn._stopTime ),
     _velocity( gn._velocity ),
     _distanceEyeCenter( gn._distanceEyeCenter ),
     _keyPressedW( gn._keyPressedW ),
     _keyPressedS( gn._keyPressedS ),
     _keyPressedA( gn._keyPressedA ),
     _keyPressedD( gn._keyPressedD ),
     _keyPressedQ( gn._keyPressedQ ),
     _keyPressedE( gn._keyPressedE ),
     _keyPressedShift_L( gn._keyPressedShift_L ),
     _slerping( gn._slerping ),
     m_world( gn.m_world )
{
}


void GodNavigation::setByMatrix( const osg::Matrixd& matrix )
{
   // set variables
   _center = matrix.getTrans();
   _rotation = matrix.getRotate();

   // fix current rotation
   if( getVerticalAxisFixed() )
      fixVerticalAxis( _center, _rotation, true );
}


void GodNavigation::setByInverseMatrix( const osg::Matrixd& matrix )
{
   setByMatrix( osg::Matrixd::inverse( matrix ) );
}


osg::Matrixd GodNavigation::getMatrix() const
{
   return osg::Matrixd::rotate( _rotation ) * osg::Matrixd::translate( _center + _rotation * osg::Vec3d( 0., 0., _distanceEyeCenter ) );
}


osg::Matrixd GodNavigation::getInverseMatrix() const
{
   return osg::Matrixd::translate( -(_center + _rotation * osg::Vec3d( 0., 0., _distanceEyeCenter )) ) * osg::Matrixd::rotate( _rotation.inverse() );
}


void GodNavigation::setTransformation( const osg::Vec3d& eye, const osg::Quat& rotation )
{
   // set variables
   _center = eye + rotation * osg::Vec3d( 0., 0., -_distanceEyeCenter );
   _rotation = rotation;

   // fix current rotation
   if( getVerticalAxisFixed() )
      fixVerticalAxis( _center, _rotation, true );
}


void GodNavigation::getTransformation( osg::Vec3d& eye, osg::Quat& rotation ) const
{
   eye = _center + _rotation * osg::Vec3d( 0., 0., _distanceEyeCenter );
   rotation = _rotation;
}


void GodNavigation::setTransformation( const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up )
{
   // set variables
   osg::Matrixd m( osg::Matrixd::lookAt( eye, center, up ) );
   _center = center;
   _rotation = m.getRotate().inverse();

   // fix current rotation
   if( getVerticalAxisFixed() )
      fixVerticalAxis( _center, _rotation, true );
}


void GodNavigation::getTransformation( osg::Vec3d& eye, osg::Vec3d& center, osg::Vec3d& up ) const
{
   center = _center;
   eye = _center + _rotation * osg::Vec3d( 0., 0., _distanceEyeCenter );
   up = _rotation * osg::Vec3d( 0.,1.,0. );
}


void GodNavigation::setVelocity( const double& velocity )
{
   _velocity = velocity;
}


double GodNavigation::getVelocity() const
{
   return _velocity;
}


bool GodNavigation::handleFrame( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*us*/ )
{
    double current_frame_time = ea.getTime();

    _delta_frame_time = current_frame_time - _last_frame_time;
    _last_frame_time = current_frame_time;

    if ( _slerping )
    {
        double timeFrame = 1. - (_stopTime - current_frame_time);

        if ( timeFrame >= 1. )
        { 
            timeFrame = 1.;
            _slerping = false;
        }

        _rotation.slerp( timeFrame, _startRotation, _stopRotation );
        performAutoZoom( timeFrame, _startDistanceEyeCenter, _stopDistanceEyeCenter );

    }

    performMovement();
    updateListener();

   return false;
}


bool GodNavigation::handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*us*/ )
{
            osg::Matrixd m( osg::Matrixd::lookAt( _homeEye, _homeCenter, _homeUp ) );
    switch( ea.getUnmodifiedKey() )
    {
        case osgGA::GUIEventAdapter::KEY_Space:
            _startRotation = _rotation;
            _stopRotation = m.getRotate().inverse();
            _startDistanceEyeCenter = _distanceEyeCenter;
            _stopDistanceEyeCenter = c_distanceEyeCenterDefault;
            _stopTime = ea.getTime() + 1.;
            _slerping = true;
            return true;

        case osgGA::GUIEventAdapter::KEY_W:
            _keyPressedW = true;
            return true;

        case osgGA::GUIEventAdapter::KEY_S:
            _keyPressedS = true;
            return true;

        case osgGA::GUIEventAdapter::KEY_A:
            _keyPressedA = true;
            return true;

        case osgGA::GUIEventAdapter::KEY_D:
            _keyPressedD = true;
            return true;

        case osgGA::GUIEventAdapter::KEY_Q:
            _keyPressedQ = true;
            return true;

        case osgGA::GUIEventAdapter::KEY_E:
            _keyPressedE = true;
            return true;

        case osgGA::GUIEventAdapter::KEY_Shift_L:
            _keyPressedShift_L = true;
            return true;

        default:
            return false;
    }

    return false;
}


bool GodNavigation::handleKeyUp( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*us*/ )
{
    switch( ea.getUnmodifiedKey() )
    {
        case osgGA::GUIEventAdapter::KEY_W:
            _keyPressedW = false;
            return true;

        case osgGA::GUIEventAdapter::KEY_S:
            _keyPressedS = false;
            return true;

        case osgGA::GUIEventAdapter::KEY_A:
            _keyPressedA = false;
            return true;

        case osgGA::GUIEventAdapter::KEY_D:
            _keyPressedD = false;
            return true;

        case osgGA::GUIEventAdapter::KEY_Q:
            _keyPressedQ = false;
            return true;

        case osgGA::GUIEventAdapter::KEY_E:
            _keyPressedE = false;
            return true;

        case osgGA::GUIEventAdapter::KEY_Shift_L:
            _keyPressedShift_L = false;
            return true;

        default:
            return false;
    }

    return false;
}


bool GodNavigation::handleMouseWheel( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*us*/ )
{
    if ( _keyPressedShift_L )
    {
        switch( ea.getScrollingMotion() )
        {
            case osgGA::GUIEventAdapter::SCROLL_UP:
                if ( _distanceEyeCenter <= 2. ) return false;
                _distanceEyeCenter -= 2.;
                return true;

            case osgGA::GUIEventAdapter::SCROLL_DOWN:
                _distanceEyeCenter += 2.;
                return true;

            default:
                return false;
        }
    } else {
        // world up vector to rotate with fixed Up vector
        osg::CoordinateFrame coordinateFrame = getCoordinateFrame( _center );
        osg::Vec3d localUp = getUpVector( coordinateFrame );

        osg::Vec3d eye = _center + _rotation * osg::Vec3d( 0., 0., _distanceEyeCenter );

        switch( ea.getScrollingMotion() )
        {
            case osgGA::GUIEventAdapter::SCROLL_UP:
                rotateYawPitch( _rotation, 0., -0.05, localUp );
                return true;

            case osgGA::GUIEventAdapter::SCROLL_DOWN:
                if ( eye.y() <= m_world->terrain->heightAt( eye.x(), eye.z() ) + 1. ) return false;
                rotateYawPitch( _rotation, 0., 0.05, localUp );
                return true;

            default:
                return false;
        }
    }
}


// This method is still under construction!
bool GodNavigation::performMovement()
{
    bool moved = false;
    double yaw = 0.;
    double velocity = _velocity * acceleratedFactor();
    double rotationSpeed = _velocity/4. * acceleratedFactor();
    
    osg::Vec3d movementDirection = osg::Vec3d( 0., 0., 0. );

    // call appropriate methods
    if ( _keyPressedW )
        calculateMovementDirectionKeyW( movementDirection );
    if ( _keyPressedS )
        calculateMovementDirectionKeyS( movementDirection );
    if ( _keyPressedA )
        calculateMovementDirectionKeyA( movementDirection );
    if ( _keyPressedD )
        calculateMovementDirectionKeyD( movementDirection );
    if ( _keyPressedQ )
        yaw += rotationSpeed;
    if ( _keyPressedE )
        yaw -= rotationSpeed;


    if ( movementDirection.length() != 0 ) {
        performMovement( movementDirection, velocity );
        moved = true;
    }

    if ( yaw != 0 ) {
        performRotationYaw( yaw );
        moved = true;
    }

    if ( moved ) {
        osg::CoordinateFrame coordinateFrame = getCoordinateFrame( _center );
        osg::Vec3d localUp = getUpVector( coordinateFrame );

        osg::Vec3d eye = _center + _rotation * osg::Vec3d( 0., 0., _distanceEyeCenter );
        while ( eye.y() <= m_world->terrain->heightAt( eye.x(), eye.z() ) )
        {
            eye = _center + _rotation * osg::Vec3d( 0., 0., _distanceEyeCenter );
            rotateYawPitch( _rotation, 0., -0.01, localUp );
        }
    }

    return moved;
}


bool GodNavigation::performMovement( const osg::Vec3d& movementDirection, const double distance )
{
    osg::Vec3d direction = movementDirection;
    direction.normalize();
    _center += (direction * distance);
    _center.y() = m_world->terrain->heightAt( _center.x(), _center.z() ) + 1.;
    return true;
}


bool GodNavigation::performRotationYaw( const double yaw )
{
    // world up vector to rotate with fixed Up vector
    osg::CoordinateFrame coordinateFrame = getCoordinateFrame( _center );
    osg::Vec3d localUp = getUpVector( coordinateFrame );

    rotateYawPitch( _rotation, yaw, 0., localUp );
    return true;
}


bool GodNavigation::performAutoZoom( const double time, const double from, const double to)
{
    _distanceEyeCenter = ( ( 1. - time ) * from ) + ( time * to );
    return true;
}


void GodNavigation::calculateMovementDirectionKeyW( osg::Vec3d& movementDirection )
{
    osg::Vec3d lookAtFront = _rotation * osg::Vec3d( 0., 0., -1. );

    movementDirection += osg::Vec3d( lookAtFront.x(), 0., lookAtFront.z() );
}


void GodNavigation::calculateMovementDirectionKeyS( osg::Vec3d& movementDirection )
{
    osg::Vec3d lookAtBack = _rotation * osg::Vec3d( 0., 0., 1. );

    movementDirection += osg::Vec3d( lookAtBack.x(), 0., lookAtBack.z() );
}


void GodNavigation::calculateMovementDirectionKeyA( osg::Vec3d& movementDirection )
{
    osg::Vec3d lookAtLeft = _rotation * osg::Vec3d( -1., 0., 0. );

    movementDirection += osg::Vec3d( lookAtLeft.x(), 0., lookAtLeft.z() );
}


void GodNavigation::calculateMovementDirectionKeyD( osg::Vec3d& movementDirection )
{
    osg::Vec3d lookAtRight = _rotation * osg::Vec3d( 1., 0., 0. );

    movementDirection += osg::Vec3d( lookAtRight.x(), 0., lookAtRight.z() );
}


double GodNavigation::acceleratedFactor()
{
    if ( _keyPressedShift_L ) return 5.;
    
    return 1.;
}


void GodNavigation::moveForward( const double distance )
{
   _center += _rotation * osg::Vec3d( 0., 0., -distance );
}


void GodNavigation::moveRight( const double distance )
{
   _center += _rotation * osg::Vec3d( distance, 0., 0. );
}


void GodNavigation::moveUp( const double distance )
{
   _center += _rotation * osg::Vec3d( 0., distance, 0. );
}

void GodNavigation::updateListener(){
    osg::Vec3d eye, center, up;
    getTransformation(eye, center, up);
    osg::Vec3d forward = eye - center;
    forward.normalize();
    up.normalize();

    m_world->soundManager->setListenerAttributes(
        { eye.x(), eye.y(), eye.z() },
        { forward.x(), forward.y(), forward.z() },
        { up.x(), up.y(), up.z() }
    );
}
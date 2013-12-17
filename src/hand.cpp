
#include "hand.h"

#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/Group>
#include <osg/ShapeDrawable>


Hand::Hand()
: m_transform(new osg::MatrixTransform())
, m_hand(new osg::Group())
{
    m_hand->addChild( osgDB::readNodeFile("data/models/hand.3DS") );
    m_transform->addChild( m_hand );

    _defaultTransform = ( osg::Matrixd::scale( 0.0005,0.0005,0.0005 ) * osg::Matrixd::rotate( 3.1415926f * 1.5, osg::Vec3(1.0, .0, .0) ) * osg::Matrixd::rotate( 3.1415926f * 1., osg::Vec3(.0, 1.0, .0) ) );

    m_transform->setMatrix( _defaultTransform );
}


Hand::~Hand()
{
}


osg::MatrixTransform * Hand::transform()
{
    return m_transform.get();
}


osg::Matrixd Hand::defaultTransform()
{
    return _defaultTransform;
}


osg::Vec3 Hand::position()
{
    osg::Matrix matrix = m_transform.get()->getMatrix();
    return osg::Vec3( matrix(3, 0), matrix(3, 1), matrix(3, 2) );
}

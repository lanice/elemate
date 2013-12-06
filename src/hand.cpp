
#include "hand.h"

#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/Group>
#include <osg/ShapeDrawable>


Hand::Hand()
: m_transform(new osg::MatrixTransform())
, m_hand(new osg::Group())
{
    m_hand->addChild( osgDB::readNodeFile("data/hand.3DS") );
    m_transform->addChild( m_hand );

    m_transform->setMatrix( m_transform->getMatrix() * osg::Matrixd::scale( 0.0005,0.0005,0.0005 ) * osg::Matrixd::rotate( 3.1415926f * 1.5, osg::Vec3(1.0, .0, .0) ) * osg::Matrixd::rotate( 3.1415926f * 1., osg::Vec3(.0, 1.0, .0) ) * osg::Matrixd::translate( 0.,2.,0. ) );
}


Hand::~Hand()
{
}


osg::MatrixTransform * Hand::transform()
{
    return m_transform.get();
}

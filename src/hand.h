
#pragma once

#include <osg/ref_ptr>
#include <osg/Matrixd>


namespace osg {
    class MatrixTransform;
    class Group;
}


class Hand 
{
public:

    Hand();
    ~Hand();

    osg::MatrixTransform * transform();
    osg::Matrixd defaultTransform();

protected:

    osg::ref_ptr<osg::MatrixTransform> m_transform;
    osg::ref_ptr<osg::Group> m_hand;

    osg::Matrixd _defaultTransform;
};

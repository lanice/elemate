#pragma once

namespace osg {
    class Matrixd;
}
namespace physx {
    class PxMat44;
}

/** converts osg to physx matrix data format without manipulating the values */
physx::PxMat44 matrixOsgToPx(const osg::Matrixd & m);

/** converts osg to physx matrix data format and mirrors along the diagonal */
physx::PxMat44 matrixOsgToPxTranslate(const osg::Matrixd & m);

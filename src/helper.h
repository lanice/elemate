#pragma once

namespace osg {
    class Matrixd;
}
namespace physx {
    class PxMat44;
}

physx::PxMat44 matrixOsgToPx(const osg::Matrixd & m);

#include "helper.h"

#include <osg/Matrixd>
#include <foundation/PxMat44.h>

physx::PxMat44 matrixOsgToPx(const osg::Matrixd & m)
{
    const osg::Matrixd::value_type * old = m.ptr();
    physx::PxReal px[16];
    for (unsigned i = 0; i < 16; ++i) {
        px[i] = physx::PxReal(old[i]);
    }
    return physx::PxMat44(px);
}

physx::PxMat44 matrixOsgToPxTranslate(const osg::Matrixd & m)
{
    const osg::Matrixd::value_type * old = m.ptr();
    physx::PxReal inv[16];
    inv[0] = physx::PxReal(old[0]); inv[1] = physx::PxReal(old[4]); inv[2] = physx::PxReal(old[8]); inv[3] = physx::PxReal(old[12]);
    inv[4] = physx::PxReal(old[1]); inv[5] = physx::PxReal(old[5]); inv[6] = physx::PxReal(old[9]); inv[7] = physx::PxReal(old[13]);
    inv[8] = physx::PxReal(old[2]); inv[9] = physx::PxReal(old[6]); inv[10] = physx::PxReal(old[10]); inv[11] = physx::PxReal(old[14]);
    inv[12] = physx::PxReal(old[3]); inv[13] = physx::PxReal(old[7]); inv[14] = physx::PxReal(old[11]); inv[15] = physx::PxReal(old[15]);

    return physx::PxMat44(inv);
}
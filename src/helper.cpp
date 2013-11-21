#include "helper.h"

#include <osg/Matrixd>
#include <foundation/PxMat44.h>


physx::PxMat44 matrixOsgToPx(const osg::Matrixd & m)
{
    const double * old = m.ptr();
    physx::PxReal inv[16];
    inv[0] = float(old[0]); inv[1] = float(old[4]); inv[2] = float(old[8]); inv[3] = float(old[12]);
    inv[4] = float(old[1]); inv[5] = float(old[5]); inv[6] = float(old[9]); inv[7] = float(old[13]);
    inv[8] = float(old[2]); inv[9] = float(old[6]); inv[10] = float(old[10]); inv[11] = float(old[14]);
    inv[12] = float(old[3]); inv[13] = float(old[7]); inv[14] = float(old[11]); inv[15] = float(old[15]);

    return physx::PxMat44(inv);
}
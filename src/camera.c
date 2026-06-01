#include <math.h>
#include "camera.h"

CameraBasis camera_basis(float lookX, float lookY, float lookZ) {
    /* Horizontal length of the look vector = |look projected onto the XZ plane|.
     * This is the magnitude of (look × worldUp), so it normalizes `right`. */
    float sideLen = sqrtf(lookZ * lookZ + lookX * lookX);
    if (sideLen < 0.0001f) sideLen = 0.0001f;   /* guard the divide (see header) */

    /* right = normalize(look × (0,1,0)) = (-lookZ, 0, lookX) / sideLen */
    float sx = -lookZ / sideLen;
    float sz =  lookX / sideLen;

    /* up = right × look.  With right.y = 0 the components simplify to: */
    CameraBasis b;
    b.rightX = sx;
    b.rightY = 0.0f;
    b.rightZ = sz;
    b.upX    = -sz * lookY;
    b.upY    =  sz * lookX - sx * lookZ;   /* equals sideLen */
    b.upZ    =  sx * lookY;
    return b;
}

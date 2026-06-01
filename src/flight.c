#include <math.h>
#include "flight.h"

void flight_reset(Flight *f) {
    f->posX = 0.0f; f->posY = 0.0f; f->posZ = 0.0f;
    f->velX = 0.0f; f->velY = 0.0f; f->velZ = 0.0f;
    f->yaw  = 0.0f; f->pitch = 0.0f;
    f->lookX = 0.0f; f->lookY = 0.0f; f->lookZ = -1.0f;
}

bool flight_update(Flight *f, const FlightConfig *cfg,
                   float stickX, float stickY,
                   bool thrustForward, bool thrustBackward) {
    /* Stick steers heading. Dividing by 85 maps the stick range to [-1, +1]. */
    f->yaw   += (stickX / 85.0f) * cfg->yawRate;
    f->pitch += (stickY / 85.0f) * cfg->pitchRate;
    if (f->pitch >  cfg->pitchMax) f->pitch =  cfg->pitchMax;
    if (f->pitch < -cfg->pitchMax) f->pitch = -cfg->pitchMax;

    /* Look direction from yaw and pitch. (0,0,-1) at yaw=pitch=0. */
    f->lookX =  sinf(f->yaw) * cosf(f->pitch);
    f->lookY =  sinf(f->pitch);
    f->lookZ = -cosf(f->yaw) * cosf(f->pitch);

    /* Thrust along the look direction. */
    bool thrusting = thrustForward || thrustBackward;
    if (thrustForward) {
        f->velX += f->lookX * cfg->thrust;
        f->velY += f->lookY * cfg->thrust;
        f->velZ += f->lookZ * cfg->thrust;
    }
    if (thrustBackward) {
        f->velX -= f->lookX * cfg->thrust;
        f->velY -= f->lookY * cfg->thrust;
        f->velZ -= f->lookZ * cfg->thrust;
    }

    /* Drag decays velocity toward zero each frame → coasting stop. */
    f->velX *= cfg->drag;
    f->velY *= cfg->drag;
    f->velZ *= cfg->drag;

    /* Clamp by vector magnitude so terminal speed is the same in any heading. */
    float spd2 = f->velX*f->velX + f->velY*f->velY + f->velZ*f->velZ;
    if (spd2 > cfg->maxSpeed * cfg->maxSpeed) {
        float inv = cfg->maxSpeed / sqrtf(spd2);
        f->velX *= inv; f->velY *= inv; f->velZ *= inv;
    }

    /* Integrate position. */
    f->posX += f->velX;
    f->posY += f->velY;
    f->posZ += f->velZ;

    /* Soft bounds: snap to the edge and kill velocity on that axis. */
    if (f->posX >  cfg->boundX)     { f->posX =  cfg->boundX;     f->velX = 0.0f; }
    if (f->posX < -cfg->boundX)     { f->posX = -cfg->boundX;     f->velX = 0.0f; }
    if (f->posY >  cfg->boundY)     { f->posY =  cfg->boundY;     f->velY = 0.0f; }
    if (f->posY < -cfg->boundY)     { f->posY = -cfg->boundY;     f->velY = 0.0f; }
    if (f->posZ >  cfg->boundZNear) { f->posZ =  cfg->boundZNear; f->velZ = 0.0f; }
    if (f->posZ <  cfg->boundZFar)  { f->posZ =  cfg->boundZFar;  f->velZ = 0.0f; }

    return thrusting;
}

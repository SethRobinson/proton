#ifndef IRRBULLET_DEBUG_MODE
    #define IRRBULLET_DEBUG_MODE
#endif

////////////////
// To-Do List //
////////////////
/*
Make ICollisionObjectCallbackSystem interface for custom user callbacks like onCollide, onLiquidUpdate, etc.
*/

/////////////////////
// Known Bugs List //
/////////////////////
/*
- In the raycast tank example, if the tank is tilted at certain angles while turning, "random" results will happen
- When using the attraction affector, Bullet will sometimes report an AABB overflow error and remove the object
- Random crashes sometimes occur in the raycast tank example
    (this has not been looked into much, but it seems to be crashing in IrrlichtDevice::run())
*/



#define IRRBULLET_VER_MAJOR 0
#define IRRBULLET_VER_MINOR 1
#define IRRBULLET_VER_MICRO 7

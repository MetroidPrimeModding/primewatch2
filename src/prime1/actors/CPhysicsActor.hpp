#ifndef DOLPHIN_EMU_CPHYSICSACTOR_H
#define DOLPHIN_EMU_CPHYSICSACTOR_H

#include "CActor.hpp"
#include "prime1/CVector3f.hpp"
#include "prime1/CQuaternion.hpp"
#include "prime1/CAABBPrimitive.hpp"

class CPhysicsActor : public CActor {
public:
    CPhysicsActor(uint32_t base_ptr, uint32_t ptr_offset = 0) : CActor(base_ptr, ptr_offset) {}

    CVector3f constantForce = CVector3f(ptr(), 0xFC);
    CVector3f angularMomentum = CVector3f(ptr(), 0x108);
    CVector3f velocity = CVector3f(ptr(), 0x138);
    CVector3f angularVelocity = CVector3f(ptr(), 0x144);
    CVector3f momentum = CVector3f(ptr(), 0x150);
    CVector3f force = CVector3f(ptr(), 0x15C);
    CVector3f impulse = CVector3f(ptr(), 0x168);
    CVector3f torque = CVector3f(ptr(), 0x174);
    CVector3f angularImpulse = CVector3f(ptr(), 0x180);
    CAABBPrimitive collisionPrimitive = CAABBPrimitive(ptr(), 0x1c0);
    CVector3f primitiveOffset = CVector3f(ptr(), 0x1E8);
    CVector3f translation = CVector3f(ptr(), 0x1F4);
    CQuaternion orientation = CQuaternion(ptr(), 0x200);
};


#endif //DOLPHIN_EMU_CPHYSICSACTOR_H

// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#ifndef __LIQUID_BODY_H_INCLUDED__
#define __LIQUID_BODY_H_INCLUDED__

#include <vector3d.h>
#include <irrTypes.h>
#include <aabbox3d.h>
#include <IrrlichtDevice.h>


class irrBulletWorld;
class IRigidBody;


/**
The ILiquidBody class is meant to approximate (not simulate) an AABB-shaped liquid body in the physical world.

This means, for example, that it is not appropriate for a realistic liquid simulation, but more for
games where accurate liquid simulation is not necessary or when visual effects are needed where liquid should affect objects.


The ILiquidBody object has two types of waves:
Local Waves and the Global Wave

Local waves are random and affect each object individually (they each get a different result).

The global wave is a "wave" that raises the upper level of the water incrementally each time the liquid
is updated. This wave is meant to simulated a kind of larger wave as in the ocean, and can be used
to emulate tidal changes by increasing the increment delay.

For simulating certain types of water bodies, a good balance between local waves and the global wave
is necessary to achieve a good effect.

The performance of the liquid body is quite good, largely due to the AABB-optimized penetration checking.

On the test platform it loses one (1) millisecond-per-frame for every 68 objects in the water.
That means 6 MPF for 410 objects.

Without AABB optimization, the same test platform got 20 MPF for 200 objects, losing 1 MPF for every 10 objects.

With 1606 objects in the water, it lost 1 MPF for every 37 objects, or 43 MPF for 1606 floating objects,
each with four buoyancy points.
*/
class ILiquidBody
{
    public:
        ILiquidBody(irrBulletWorld* const world, const irr::core::vector3df& pos, const irr::core::aabbox3df& aabb,
            irr::f32 waveFrequency=40000.0f, irr::f32 density=0.4f, bool makeInfinite=false);
        ~ILiquidBody();

        ///! For internal use only
        void updateLiquidBody();

        /// Sets the frequency at which the liquid level is updated
        void setGlobalWaveUpdateFrequency(irr::f32 frequency) { GlobalWaveUpdateFrequency = frequency; };

        void setMaxGlobalWaveHeight(irr::f32 max) { MaxGlobalWaveHeight = max; };

        void setMinGlobalWaveHeight(irr::f32 min) { MinGlobalWaveHeight = min; };

        /// Sets the distance that the liquid level rises or lowers each time it's updated
        void setGlobalWaveChangeIncrement(irr::f32 increment) { GlobalWaveChangeIncrement = increment; };

        /// Sets the effective volume of the liquid
        void setLiquidExtents(const irr::core::aabbox3df& newExtents) { LiquidExtents = newExtents; };

        void setDebugDrawEnabled(bool b) { DebugDraw = b; };

        /// If enabled, the liquid level will rise and fall
        void setLocalWavesEnabled(bool b) { EnableLocalWaves = b; };

        void setGlobalWaveEnabled(bool b) { EnableGlobalWave = b; };

        /// liquid density affects how buoyant objects will be and how well they are held up
        void setLiquidDensity(irr::f32 density) { LiquidDensity = density; };

        /**
            This sets the "friction" of the liquid.

            For each buoyancy point on an object that is under liquid, the velocity of that local point
            is taken, its direction inverted, and then multiplied by the liquid friction.

            This should be related to the liquid density, but for ease-of-use it is separated into friction.
        */
        void setLiquidFriction(irr::f32 friction) { LiquidFriction = friction; };

        /// Sets the direction of the liquid's current; This is useful for lakes and such.
        void setCurrentDirection(const irr::core::vector3df& direction) { CurrentDirection = direction; };

        /// Sets the speed of the liquid's current
        void setCurrentSpeed(irr::f32 speed) { CurrentSpeed = speed; };

        /// Sets the min, max, and multiplier values of random "waves" that affect Local objects in the liquid
        void setLocalWaveValues(irr::u32 max, irr::u32 min, irr::f32 multiplier=0.1f)
        {
            LocalWaveForceMax=max;
            LocalWaveForceMin=min;
            LocalWaveForceMultiplier=multiplier;
        };

        /**
            Like liquid friction, this should be related to liquid density, but for ease-of-use it is separated.

            The angular limitation is the value by which the angular velocity of any object in the liquid is multiplied.

            To have no effect on angular velocity of objects in the liquid, set this value to 1.0.
            By default the angular limitation is 0.995.
        */
        void setAngularLimitation(irr::f32 limitation) { AngularLimitation = limitation; };

        void setInfinite(bool b) { Infinite = b; };

        /// Infinite depth only works if the water is infinite, otherwise it will have no effect
        void setInfiniteDepth(bool b) { InfiniteDepth = b; };

        /// If true, all bodies will be activated if they are about to be deactivated or are already sleeping
        void setForceActivationEnabled(bool b) { ForceActivation = b; };




        irr::u32 getUniqueID() const { return UniqueID; };

        irr::f32 getGlobalWaveUpdateFrequency() const { return GlobalWaveUpdateFrequency; };

        irr::f32 getMaxGlobalWaveHeight() const { return MaxGlobalWaveHeight; };

        irr::f32 getMinGlobalWaveHeight() const { return MinGlobalWaveHeight; };

        irr::f32 getGlobalWaveChangeIncrement() const { return GlobalWaveChangeIncrement; };

        irr::f32 getStaticLiquidLevel() const { return LiquidLevel; };

        irr::f32 getLiquidDensity() const { return LiquidDensity; };

        irr::f32 getLiquidFriction() const { return LiquidFriction; };

        const irr::core::aabbox3df& getLiquidExtents() const { return LiquidExtents; };

        bool isDebugDrawEnabled() const { return DebugDraw; };

        bool areLocalWavesEnabled() const { return EnableLocalWaves; };

        bool isGlobalWaveEnabled() const { return EnableGlobalWave; };

        /// Global wave
        bool isLiquidRising() const { return LiquidRising; };

        const irr::core::vector3df& getCurrentDirection() const { return CurrentDirection; };

        /// @return The speed of the water's current
        irr::f32 getCurrentSpeed() const { return CurrentSpeed; };

        irr::u32 getLocalWaveForceMax() const { return LocalWaveForceMax; };

        irr::u32 getLocalWaveForceMin() const { return LocalWaveForceMin; };

        /// @return The value by which all "local wave" heights will be multiplied
        irr::f32 getLocalWaveForceMultiplier() const { return LocalWaveForceMultiplier; };

        irr::f32 getAngularLimitation() const { return AngularLimitation; };

        /// @return Whether or not the water is infinite on the X and Z axes
        bool isInfinite() const { return Infinite; };

        bool isInfiniteDepth() const { return InfiniteDepth; };

        /// If enabled, all bodies in the water will be awakened if they are deactivated or want deactivation
        bool isForceActivationEnabled() const { return ForceActivation; };


    private:
        irrBulletWorld* DynamicsWorld;
        irr::IrrlichtDevice* Device;
        irr::core::aabbox3df LiquidExtents;
        irr::core::vector3df CurrentDirection;
        irr::core::vector3df MaxEdge;
        irr::core::vector3df MinEdge;
        irr::f32 LastGlobalWaveUpdateTime;
        irr::f32 GlobalWaveUpdateFrequency;
        irr::f32 LiquidLevel;
        irr::f32 LiquidDensity;
        irr::f32 MaxGlobalWaveHeight;
        irr::f32 MinGlobalWaveHeight;
        irr::f32 GlobalWaveChangeIncrement;
        irr::f32 LiquidFriction;
        irr::f32 AngularLimitation;
        irr::f32 CurrentSpeed;
        irr::u32 LocalWaveForceMax;
        irr::u32 LocalWaveForceMin;
        irr::f32 LocalWaveForceMultiplier;
        irr::u32 UniqueID;
        bool DebugDraw;
        bool EnableLocalWaves;
        bool EnableGlobalWave;
        bool LiquidRising;
        bool Infinite;
        bool InfiniteDepth;
        bool ForceActivation;

        ///! For internal use only
        void affectRigidBody(IRigidBody* const body);
};

#endif // __LIQUID_BODY_H_INCLUDED__

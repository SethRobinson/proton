// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#ifndef __COLLISION_SHAPE_H_INCLUDED__
#define __COLLISION_SHAPE_H_INCLUDED__

#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include <vector3d.h>
#include <irrTypes.h>
#include <irrString.h>
#include "irrbulletcommon.h"

class btVector3;
class btTransform;
namespace irr
{
    namespace scene
    {
        class ISceneNode;
    }
}


enum ECollisionShapeType
{
    ECST_BOX,
    ECST_CONE,
    ECST_SPHERE,
    ECST_BVHTRIMESH,
    ECST_CONVEXHULL,
    ECST_GIMPACT
};

enum EScalingPair
{
    ESP_BOTH,
    ESP_COLLISIONSHAPE,
    ESP_VISUAL
};


/// The collision shape defines the geometry of an object, its mass, and its inertia.
class ICollisionShape
{
public:
    ICollisionShape();

    virtual ~ICollisionShape();

    /*!
        Sets the scaling of the collision shape in local space.
        @param scaling How much to scale the shape.
        @param esp Whether to scale only the collision shape, the scene node, or both.
    */
    void setLocalScaling(const irr::core::vector3df &scaling, const EScalingPair esp);

    /*!
        Sets the margin of the shape. (How much of a "buffer zone" the object has).
        Useful to somewhat avoid tunneling, and catch collisions further from the actual geometry.
        (Note: shapes such as sphere and box actually subtract the margin from the actual geometry,
        while triangle meshes and the like add it to the geometry.)
    */
    void setMargin(const irr::f32 margin) { shape->setMargin(margin); };


    /// Calculates the local inertia of this shape (How much the object resists changes to its current state of motion).
    void calculateLocalInertia(irr::f32 Mass, const irr::core::vector3df &inertia);

    void removeNode() { node->remove(); };

    const irr::core::vector3df getLocalScaling() const;

    const irr::core::stringc getName() const { return shape->getName(); };

    ECollisionShapeType getShapeType() const { return type; };

    irr::f32 getMargin() const { return shape->getMargin(); };

    /// Only for internal use by irrBullet.
    const irr::core::vector3df getLocalInertia() const { return localInertia; };

    irr::f32 getMass() const {return mass;};

    btCollisionShape* const getPointer() {return shape;};

    irr::scene::ISceneNode* const getSceneNode() const {return node;};

    bool isPolyhedral() const { return shape->isPolyhedral(); };
    bool isConvex() const { return shape->isConvex(); };
    bool isConcave() const { return shape->isConcave(); };
    bool isCompound() const { return shape->isCompound(); };
    bool isInfinite() const { return shape->isInfinite(); };


protected:
    ECollisionShapeType type;
    irr::scene::ISceneNode *node;
    btCollisionShape *shape;

    irr::f32 mass;
    irr::core::vector3df localInertia;

    // Functions
    virtual void createShape();
};


#endif // __COLLISION_SHAPE_H_INCLUDED__

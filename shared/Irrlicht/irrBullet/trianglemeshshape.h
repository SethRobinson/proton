// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#ifndef __TRIANGLE_MESH_SHAPE_H_INCLUDED__
#define __TRIANGLE_MESH_SHAPE_H_INCLUDED__

#include "collisionshape.h"
#include "BulletCollision/CollisionShapes/btTriangleMesh.h"
#include "irrbulletcommon.h"

namespace irr
{
    namespace scene
    {
        class IMesh;
    }
}

/// The triangle mesh shape, as the name suggests, is made up of triangles extracted from a mesh.
/*!
    @see IGImpactMeshShape
    @see IBvhTriangleMeshShape
*/
class ITriangleMeshShape : public ICollisionShape
{
    public:
        ITriangleMeshShape();
        virtual ~ITriangleMeshShape();

    protected:
        virtual void createShape(irr::scene::IMesh* const collMesh);

        /// @return The btTriangleMesh created by extracting the geometry from an Irrlicht IMesh.
        btTriangleMesh *getTriangleMesh(irr::scene::IMesh* const mesh);
};

#endif // __TRIANGLE_MESH_SHAPE_H_INCLUDED__

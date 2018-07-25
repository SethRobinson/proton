// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#ifndef __BVH_TRIANGLE_MESH_SHAPE_H_INCLUDED__
#define __BVH_TRIANGLE_MESH_SHAPE_H_INCLUDED__

#include "trianglemeshshape.h"


/// The BVH triangle mesh shape is useful for static triangle mesh shapes, such as game levels.
class IBvhTriangleMeshShape : public ITriangleMeshShape
{
    public:
        IBvhTriangleMeshShape(irr::scene::ISceneNode *n, irr::scene::IMesh *collMesh, irr::f32 m);

        virtual ~IBvhTriangleMeshShape();


    protected:
        virtual void createShape(irr::scene::IMesh* const collMesh);
};

#endif // __BVH_TRIANGLE_MESH_SHAPE_H_INCLUDED__

// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#ifndef __SOFTBODY_H_INCLUDED__
#define __SOFTBODY_H_INCLUDED__

#include "collisionobject.h"
#include "BulletSoftBody/btSoftBodyHelpers.h"
#include "BulletSoftBody/btSoftBody.h"
#include <irrTypes.h>
#include <map>
#include <vector>
#include <irrArray.h>
#include <S3DVertex.h>
#include <IMeshBuffer.h>

namespace irr
{
    namespace scene
    {
        class IMeshSceneNode;
        class IMesh;
    }
}

class irrBulletWorld;
class IRigidBody;

enum ESoftBodyCollisionFlag
{
    ESBCM_RIGID_VERSUS_SOFT_MASK = 0x000f,
    ESBCM_SDF_RIGID_VERSUS_SOFT = 0x0001,
    ESBCM_CLUSTER_RIGID_VERSUS_SOFT = 0x0002,
    ESBCM_SOFT_VERSUS_SOFT_MASK = 0x0030,
    ESBCM_VERTEX_FACE_SOFT_VERSUS_SOFT = 0x0010,
    ESBCM_CLUSTER_SOFT_VERSUS_SOFT = 0x0020,
    ESBCM_CLUSTER_SELF_COLLISION = 0x0040,
    ESBCM_DEFAULT = ESBCM_SDF_RIGID_VERSUS_SOFT,
    ESBCM_END
};

enum ESoftBodyAeroModel
{
    ESBAM_VERTEX_POINT = 0,
    ESBAM_VERTEX_TWO_SIDED = 1,
    ESBAM_VERTEX_ONE_SIDED = 2,
    ESBAM_FACE_TWO_SIDED = 3,
    ESBAM_FACE_ONE_SIDED = 4,
    ESBAM_END = 5
};


struct SSoftBodyConfiguration
{
    void importData(const btSoftBody::Config& cfg)
    {
        velocityCorrectionFactor = cfg.kVCF;
        dampingCoefficient = cfg.kDP;
        dragCoefficient = cfg.kDG;
        liftCoefficient = cfg.kLF;
        pressureCoefficient = cfg.kPR;
        volumeConversationCoefficient = cfg.kVC;
        dynamicFrictionCoefficient = cfg.kDF;
        poseMatchingCoefficient = cfg.kMT;
        rigidContactsHardness = cfg.kCHR;
        kineticContactsHardness = cfg.kKHR;
        softContactsHardness = cfg.kSHR;
        anchorsHardness = cfg.kAHR;
        softRigidHardnessCL = cfg.kSRHR_CL;
        softKineticHardnessCL = cfg.kSKHR_CL;
        softSoftHardnessCL = cfg.kSSHR_CL;
        kSR_SPLT_CL = cfg.kSR_SPLT_CL;
        kSK_SPLT_CL = cfg.kSK_SPLT_CL;
        kSS_SPLT_CL = cfg.kSS_SPLT_CL;
        maxVolume = cfg.maxvolume;
        timeScale = cfg.timescale;
        velocitiesSolverIterations = cfg.viterations;
        positionsSolverIterations = cfg.piterations;
        driftSolverIterations = cfg.diterations;
        clusterSolverIterations = cfg.citerations;
        collisionFlags = cfg.collisions;
        aeroModel = static_cast<ESoftBodyAeroModel>(cfg.aeromodel);
    }

    void exportData(btSoftBody::Config& cfg)
    {
        cfg.kVCF = velocityCorrectionFactor;
        cfg.kDP = dampingCoefficient;
        cfg.kDG = dragCoefficient;
        cfg.kLF = liftCoefficient;
        cfg.kPR = pressureCoefficient;
        cfg.kVC = volumeConversationCoefficient;
        cfg.kDF = dynamicFrictionCoefficient;
        cfg.kMT = poseMatchingCoefficient;
        cfg.kCHR = rigidContactsHardness;
        cfg.kKHR = kineticContactsHardness;
        cfg.kSHR = softContactsHardness;
        cfg.kAHR = anchorsHardness;
        cfg.kSRHR_CL = softRigidHardnessCL;
        cfg.kSKHR_CL = softKineticHardnessCL;
        cfg.kSSHR_CL = softSoftHardnessCL;
        cfg.kSR_SPLT_CL = kSR_SPLT_CL;
        cfg.kSK_SPLT_CL = kSK_SPLT_CL;
        cfg.kSS_SPLT_CL = kSS_SPLT_CL;
        cfg.maxvolume = maxVolume;
        cfg.timescale = timeScale;
        cfg.viterations = velocitiesSolverIterations;
        cfg.piterations = positionsSolverIterations;
        cfg.diterations = driftSolverIterations;
        cfg.citerations = clusterSolverIterations;
        cfg.collisions = collisionFlags;
        cfg.aeromodel = static_cast<btSoftBody::eAeroModel::_>(aeroModel);
    }

    irr::f32 velocityCorrectionFactor;
    irr::f32 dampingCoefficient;
    irr::f32 dragCoefficient;
    irr::f32 liftCoefficient;
    irr::f32 pressureCoefficient;
    irr::f32 volumeConversationCoefficient;
    irr::f32 dynamicFrictionCoefficient;
    irr::f32 poseMatchingCoefficient;
    irr::f32 rigidContactsHardness;
    irr::f32 kineticContactsHardness;
    irr::f32 softContactsHardness;
    irr::f32 anchorsHardness;
    irr::f32 softRigidHardnessCL;
    irr::f32 softKineticHardnessCL;
    irr::f32 softSoftHardnessCL;
    irr::f32 kSR_SPLT_CL; // Soft vs rigid impulse split [0,1] (cluster only)
    irr::f32 kSK_SPLT_CL; // Soft vs rigid impulse split [0,1] (cluster only)
    irr::f32 kSS_SPLT_CL; // Soft vs rigid impulse split [0,1] (cluster only)
    irr::f32 maxVolume;
    irr::f32 timeScale;
    irr::u32 velocitiesSolverIterations;
    irr::u32 positionsSolverIterations;
    irr::u32 driftSolverIterations;
    irr::u32 clusterSolverIterations;
    irr::u32 collisionFlags;
    ESoftBodyAeroModel aeroModel;
};


class ISoftBody : public ICollisionObject
{
    public:
        ISoftBody(irrBulletWorld* const world, irr::scene::IMeshSceneNode* const Node);
        ~ISoftBody();

        /// @note Internal use only!
        void updateSoftBody();

        /// @note Internal use only!
        void updateMeshBuffer(irr::scene::IMeshBuffer* const mb, irr::s32& count);

        /// After making changes to the softbody configuration from getConfiguration(), this must be called for them to take effect
        void updateConfiguration();

        void randomizeConstraints();

        /// Constrains bending between two vertices connected by an edge with the given distance
        void generateBendingConstraints(irr::u32 distance);

        void setPose(bool volume, bool frame);

        /*!
            @param node The vertex ID to anchor
            @param body The rigid body to append the vertex to
            @param disableCollisionBetweenLinkedBodies If true, collisions between the softbody and the rigid body will be disabled
        */
        void appendAnchor(irr::u32 node, IRigidBody* const body, bool disableCollisionBetweenLinkedBodies=true);

        /// Adds a force(force) to the vertex with the given ID (node)
        void addForce(const irr::core::vector3df& force, irr::u32 node);

        /// Adds a velocity to the entire softbody
        void addVelocity(const irr::core::vector3df& velocity);

        /// Sets the velocity of the entire softbody
        void setVelocity(const irr::core::vector3df& velocity);

        /// Adds a force to the entire softbody
        void addForce(const irr::core::vector3df& force);

        void setTotalMass(irr::f32 mass, bool fromfaces=false);

        void setVolumeDensity(irr::f32 density);

        void setVolumeMass(irr::f32 mass);

        void setTotalDensity(irr::f32 density);

        /// Adds a softbody collision flag to the collision flags
        void addCollisionFlag(ESoftBodyCollisionFlag flag) { getPointer()->m_cfg.collisions += flag; };

        void removeCollisionFlag(ESoftBodyCollisionFlag flag) { getPointer()->m_cfg.collisions -= flag; };

        void generateClusters(irr::u32 count, irr::u32 maxIterations=8192) { getPointer()->generateClusters(count, maxIterations); };





        btSoftBody* const getPointer() const { return static_cast<btSoftBody*>(object); };

        irr::u32 getNumVertices() const { return vertexCount; };

        irr::scene::IMeshSceneNode* const getSceneNode() const { return node; };

        void setScale(const irr::core::vector3df& newscale);

        const irr::core::vector3df& getScale() const { return scale; };

        SSoftBodyConfiguration& getConfiguration() { return configuration; };

        bool checkLink(irr::u32 node0, irr::u32 node1);

    private:
        void createShape(irr::scene::IMesh* const collMesh);
        void configureSoftBody();
        irr::scene::IMeshSceneNode* node;
        irr::core::vector3df scale;

        SSoftBodyConfiguration configuration;

        irr::f32 *vertices;
        int *indices;
        int indexCount, vertexCount;
        int realVertexCount;

        std::map<int, int> MeshMap;
        std::map<btSoftBody::Node*, int> node_map;
        std::vector<int> m_indices;
        std::map<int, btSoftBody::Node*> m_vertices;
};

#endif // __SOFTBODY_H_INCLUDED__

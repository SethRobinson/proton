// Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
// This file is part of the "irrBullet" Bullet physics extension library and wrapper.
// For conditions of distribution and use, see license in irrbullet.h
// The above copyright notice and its accompanying information must remain here.

#include "softbody.h"
#include <IMesh.h>
#include <IMeshBuffer.h>
#include <IMeshSceneNode.h>
#include <vector3d.h>
#include <S3DVertex.h>
#include <ISceneManager.h>
#include <IMeshManipulator.h>
#include "bulletworld.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;

ISoftBody::ISoftBody(irrBulletWorld* const world, IMeshSceneNode* const Node)
{
    objectType = ECOT_SOFT_BODY;

    dynamicsWorld = world;

    node = Node;

    createShape(Node->getMesh());
    configureSoftBody();

    collID = new SCollisionObjectIdentification();
    collID->setCollisionObject(this);
    collID->setName("RigidBody");

    // The following line causes a crash
    object->setUserPointer(collID);

    //enable cluster collision between soft body and rigid body
    //getPointer()->m_cfg.collisions += btSoftBody::fCollision::CL_RS;
    //enable cluster collision between soft body and soft body
    //getPointer()->m_cfg.collisions += btSoftBody::fCollision::CL_SS;
}

// ISoftBody::createShape() is taken from code found on the Irrlicht forum
void ISoftBody::createShape(IMesh* const collMesh)
{
    int cMeshBuffer, j;
    IMeshBuffer *mb;
    video::S3DVertex* mb_vertices;
    u16* mb_indices;


    std::map<int, int> index_map;
    std::map<int, int> bullet_map;
    std::map<int, S3DVertex> vertex_map;
    int count = 0;
    indexCount = 0;

    vertexCount = 0;

    for(cMeshBuffer=0; cMeshBuffer < 1; cMeshBuffer++)
    {
        //printf("Loading new mesh buffer for softbody.\n");
        mb = collMesh->getMeshBuffer(cMeshBuffer);
        mb_vertices = (irr::video::S3DVertex*)mb->getVertices();
        mb_indices = mb->getIndices();

        indexCount += mb->getIndexCount();
        vertexCount += mb->getVertexCount();


        for(int i=0; i<mb->getIndexCount(); i++)
        {
            int iIndex = mb_indices[i];
            vector3df iVertex = mb_vertices[iIndex].Pos;
            bool isFirst = true;
            for(int j=0; j<i; j++)
            {
                int jIndex = mb_indices[j];
                vector3df jVertex = mb_vertices[jIndex].Pos;
                if (iVertex == jVertex)
                {
                    index_map.insert(std::make_pair(i, j));
                    isFirst = false;
                    break;
                }
            }
            // ???????Bullet??Index??????
            if(isFirst)
            {
                // Irrlicht?Index??????Index
                index_map.insert(std::make_pair(i, i));
                // ?????Index????Index
                bullet_map.insert(std::make_pair(i, count));
                // ??Index?????????
                vertex_map.insert(std::make_pair(count, mb_vertices[iIndex]));
                count++;
            }
        }
    }

    indices = new int[indexCount];
    vertexCount = vertex_map.size();
    vertices = new btScalar[vertexCount*3];

    for(j=0; j<indexCount; j++)
    {
        int index1 = index_map.find(j)->second;
        int index2 = bullet_map.find(index1)->second;
        indices[j]   = index2;
    }

    for(j=0; j<vertexCount; j++)
    {
        vertices[3*j] =   vertex_map[j].Pos.X;
        vertices[3*j+1] = vertex_map[j].Pos.Y;
        vertices[3*j+2] = -vertex_map[j].Pos.Z;
    }

    //std::cout << "create softbody" << std::endl;
    object = btSoftBodyHelpers::CreateFromTriMesh(
        dynamicsWorld->getSoftBodyWorldInfo(), vertices,indices, indexCount/3);

    //std::cout << "create map" << std::endl;
    for(int i=0; i<getPointer()->m_faces.size(); i++)
    {
        btSoftBody::Face face = getPointer()->m_faces[i];

        for(int j=0; j<3; j++)
        {
            if(node_map.find(face.m_n[j]) == node_map.end())
            {
                node_map.insert(std::make_pair(face.m_n[j], node_map.size()));
            }
        }

        for(int j=0; j<3; j++)
        {
            m_indices.push_back(node_map.find(face.m_n[j])->second);
        }
    }

    // Reverse node->index to index->node (should be unique on both sides)
    std::map<btSoftBody::Node*, int>::const_iterator node_iter;
    for(node_iter = node_map.begin(); node_iter != node_map.end(); ++node_iter)
    {
        m_vertices.insert(std::make_pair(node_iter->second, node_iter->first));
    }

    //std::cout << "update Irrlicht vertices" << std::endl;
    std::map<int, btSoftBody::Node*>::const_iterator it;


    for(int i=0; i<mb->getVertexCount(); i++)
    {
        for(it=m_vertices.begin(); it != m_vertices.end(); ++it)
        {
            int v_index = it->first;
            btSoftBody::Node* node = it->second;
            if(node->m_x.x() == mb_vertices[i].Pos.X &&
                node->m_x.y() == mb_vertices[i].Pos.Y &&
                node->m_x.z() == mb_vertices[i].Pos.Z)
            {
                MeshMap.insert(std::make_pair(i, v_index));
                break;
            }
        }
    }
}

void ISoftBody::configureSoftBody()
{
    configuration.importData(getPointer()->m_cfg);
    scale = node->getScale();

    // Set defaults
    getPointer()->m_cfg.kDP = 0.0;  // Damping coefficient [0,1]
    getPointer()->m_cfg.kDF = 0.2;  // Dynamic friction coefficient [0,1]
    getPointer()->m_cfg.kMT = 0.02; // Pose matching coefficient [0,1]
    getPointer()->m_cfg.kCHR = 1.0; // Rigid contacts hardness [0,1]
    getPointer()->m_cfg.kKHR = 0.8; // Kinetic contacts hardness [0,1]
    getPointer()->m_cfg.kSHR = 1.0; // Soft contacts hardness [0,1]
    getPointer()->m_cfg.piterations=2;
    getPointer()->m_materials[0]->m_kLST = 0.8;
    getPointer()->m_materials[0]->m_kAST = 0.8;
    getPointer()->m_materials[0]->m_kVST = 0.8;
    getPointer()->scale(irrlichtToBulletVector(scale));
    setPose(true, false);
    generateBendingConstraints(2);
    randomizeConstraints();

    // Reset the scale of the node, since the scale of the softbody now takes over
    node->setScale(vector3df(1,1,1));


    // Transform the softbody to be positioned and rotated to the same values as the scene node
    worldTransform.setTranslation(node->getPosition());
    worldTransform.setRotationDegrees(node->getRotation());

    //btTransformFromIrrlichtMatrix(worldTransform, internalTransform);

    //getPointer()->transform(internalTransform);

    setWorldTransform(worldTransform);
}

void ISoftBody::setVolumeDensity(irr::f32 density)
{
    getPointer()->setVolumeDensity(density);
}

void ISoftBody::setVolumeMass(irr::f32 mass)
{
    getPointer()->setVolumeMass(mass);
}

void ISoftBody::setTotalDensity(irr::f32 density)
{
    getPointer()->setTotalDensity(density);
}

void ISoftBody::setTotalMass(irr::f32 mass, bool fromfaces)
{
    getPointer()->setTotalMass(mass, fromfaces);
}

void ISoftBody::setVelocity(const vector3df& velocity)
{
    getPointer()->setVelocity(irrlichtToBulletVector(velocity));
}

void ISoftBody::addVelocity(const vector3df& velocity)
{
    getPointer()->addVelocity(irrlichtToBulletVector(velocity));
}

void ISoftBody::addForce(const vector3df& force, irr::u32 node)
{
    getPointer()->addForce(irrlichtToBulletVector(force), node);
}

void ISoftBody::addForce(const vector3df& force)
{
    getPointer()->addForce(irrlichtToBulletVector(force));
}

bool ISoftBody::checkLink(irr::u32 node0, irr::u32 node1)
{
    return getPointer()->checkLink(node0, node1);
}

void ISoftBody::appendAnchor(irr::u32 node, IRigidBody* const body, bool disableCollisionBetweenLinkedBodies)
{
    getPointer()->appendAnchor(node, body->getPointer(), disableCollisionBetweenLinkedBodies);
}

void ISoftBody::randomizeConstraints()
{
    getPointer()->randomizeConstraints();
}

void ISoftBody::generateBendingConstraints(irr::u32 distance)
{
    getPointer()->generateBendingConstraints(distance);
}

void ISoftBody::setPose(bool volume, bool frame)
{
    getPointer()->setPose(volume, frame);
}

void ISoftBody::updateConfiguration()
{
    configuration.exportData(getPointer()->m_cfg);
}

void ISoftBody::setScale(const vector3df& newscale)
{
    scale = newscale;
    getPointer()->scale(irrlichtToBulletVector(scale));
}

void ISoftBody::updateSoftBody()
{
    // Update the node position
    getWorldTransform();
    node->setPosition(worldTransform.getTranslation());
    node->setRotation(worldTransform.getRotationDegrees());

    // Update the vertices
    int cMeshBuffer, j;
    IMeshBuffer *mb;
    IMesh* collMesh = node->getMesh();
    s32 count = -1;

    for(cMeshBuffer=0; cMeshBuffer < 1; cMeshBuffer++)
    {
        mb = collMesh->getMeshBuffer(cMeshBuffer);

        updateMeshBuffer(mb, count);
        mb->recalculateBoundingBox();
    }
    // Update the normals so they're not messed up by the soft body calculations
    node->getSceneManager()->getMeshManipulator()->recalculateNormals(collMesh, false, false);
}

void ISoftBody::updateMeshBuffer(IMeshBuffer* mb, s32& count)
{
    S3DVertex* mb_vertices = (irr::video::S3DVertex*)mb->getVertices();
    u16* mb_indices = mb->getIndices();

    for(int i=0; i<mb->getVertexCount(); i++)
    {
        int index = MeshMap.find(i)->second;
        btSoftBody::Node* vNode = m_vertices.find(index)->second;
        mb_vertices[i].Pos.X = vNode->m_x.x();
        mb_vertices[i].Pos.Y = vNode->m_x.y();
        mb_vertices[i].Pos.Z = vNode->m_x.z();
    }
}

ISoftBody::~ISoftBody()
{
    /*btSoftBody* softBody = getPointer();
    if(softBody)
    {
        dynamicsWorld->getPointer()->removeSoftBody(softBody);
    }*/

    delete[] indices;
    delete[] vertices;

    if(IncludeNodeOnRemoval)
        node->remove();

    if(collID)
        delete collID;
}

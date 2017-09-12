
/*MIT License

Copyright (c) 2017 Colin Courtney

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#pragma once

#include <entityx/entityx.h>
#include <algorithm>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <LinearMath/btDefaultMotionState.h>

#define MAX_SUB_STEPS 5
#define FIXED_TIME_STEP 1.0f/60.0f

struct RigidBody{
    RigidBody(){
        mass = 0.0f;
        shape = NULL;
    }
    RigidBody(std::string name,btScalar mass,btVector3 origin,btCollisionShape* shape){
        this->name = name;
        this->mass = mass;
        this->origin = origin;
        this->shape = shape;
    }
    btCollisionShape* shape;
    btScalar mass;
    btVector3 origin;
    btRigidBody* body;
    std::string name;
};

class PhysicsSystem: public entityx::System<PhysicsSystem>,public entityx::Receiver<PhysicsSystem>{
    btDiscreteDynamicsWorld* world;
    btSequentialImpulseConstraintSolver* solver;
    btCollisionDispatcher* dispatcher;
    btDefaultCollisionConfiguration* config;
    btBroadphaseInterface* broadphase;
    std::vector<btCollisionShape*> shapes;
public:
    void configure(entityx::EventManager& events) override{
        events.subscribe<entityx::ComponentAddedEvent<RigidBody>>(*this);
    }
    PhysicsSystem(){
        ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
        config = new btDefaultCollisionConfiguration();

        ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
        dispatcher = new btCollisionDispatcher(config);

        ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
        broadphase = new btDbvtBroadphase();

        ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
        solver = new btSequentialImpulseConstraintSolver;

        world = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,config);
        world->setGravity(btVector3(0, -10, 0));        
    }

    void receive(const entityx::ComponentAddedEvent<RigidBody>& event){
        RigidBody* rb = (RigidBody*)event.component.get();
        if(rb->shape == NULL) return;
        //SDL_Log("COMPONENT ADDED: %s",rb->name.c_str());  

        //Set the body's position.
        shapes.push_back(rb->shape);
        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(rb->origin);

        //If this is a dynamic body, calculate its local inertia.
        bool isDynamic = (rb->mass != 0.f);
        btVector3 localInertia(0, 0, 0);
        if (isDynamic) rb->shape->calculateLocalInertia(rb->mass, localInertia);

        //Set motion state for carrying out the simulation.
        btRigidBody::btRigidBodyConstructionInfo rbInfo(rb->mass, new btDefaultMotionState(transform), rb->shape,
                                                            localInertia);
        //Add the rigid body to the scene.
        rb->body = new btRigidBody(rbInfo);
        world->addRigidBody(rb->body);
    }
    ~PhysicsSystem(){
        //Delete rigid bodies
        for (int i = world->getNumCollisionObjects() - 1; i >= 0; i--) {
            btCollisionObject *obj = world->getCollisionObjectArray()[i];
            btRigidBody *body = btRigidBody::upcast(obj);
            if (body && body->getMotionState()) delete body->getMotionState();
            world->removeCollisionObject(obj);
            delete obj;
        }

        //Delete collision shapes
        for(btCollisionShape* shape:shapes) delete shape;

        delete world;
        delete solver;
        delete dispatcher;
        delete config;
        delete broadphase;
    }
    void update(entityx::EntityManager& entities,entityx::EventManager& events,entityx::TimeDelta dt) override{
        world->stepSimulation(std::min(1.0/30.0,(double)dt),MAX_SUB_STEPS,FIXED_TIME_STEP);
    }
};
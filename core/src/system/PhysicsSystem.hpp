
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
    btDefaultMotionState* motionState;
    std::string name;
};

class PhysicsSystem: public entityx::System<PhysicsSystem>,public entityx::Receiver<PhysicsSystem>{
    btSequentialImpulseConstraintSolver* solver;
    btCollisionDispatcher* dispatcher;
    btDefaultCollisionConfiguration* config;
    btBroadphaseInterface* broadphase;
    std::vector<btCollisionShape*> shapes;
public:
    btDiscreteDynamicsWorld* world;
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
        try{
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
            rb->motionState = new btDefaultMotionState(transform);
            btRigidBody::btRigidBodyConstructionInfo rbInfo(rb->mass, rb->motionState, rb->shape,
                                                                localInertia);
            //Add the rigid body to the scene.
            rb->body = new btRigidBody(rbInfo);
            world->addRigidBody(rb->body);
        }catch(std::exception& e){
            std::cerr << "Exception caught while adding a rigid body: " << e.what() << std::endl;
        }
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
        //for(btCollisionShape* shape:shapes) delete shape;

        delete world;
        delete solver;
        delete dispatcher;
        delete config;
        delete broadphase;
    }
    void update(entityx::EntityManager& entities,entityx::EventManager& events,entityx::TimeDelta dt) override{
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        world->stepSimulation(std::min(1.0f/30.0f,(float)dt),MAX_SUB_STEPS,FIXED_TIME_STEP);/*->stepSimulation( 1.0f / 60.0f, 0 );*/
        //SDL_Log("PHYSICS TIME: %f",(float)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - now).count());
    }
};
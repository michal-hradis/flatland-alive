//
// Created by ihradis on 3/1/21.
//
#include <box2d/box2d.h>
#include <opencv2/core.hpp>
#include "World.h"

void createVerticalWall(b2World &world, const float xpos, const float ypos, const float size){
    BasicObject* props = new BasicObject();
    props->color = cv::Scalar(128, 128, 128);

    for(float y = 0; y  < size + 1; y += 2) {
        b2BodyDef bodyDef;
        bodyDef.position.Set(xpos, ypos + y);
        b2Body *groundBody = world.CreateBody(&bodyDef);
        groundBody->GetUserData().pointer = (uintptr_t) props;
        b2PolygonShape groundBox;
        groundBox.SetAsBox(1, 1);
        groundBody->CreateFixture(&groundBox, 0.0f);
    }
}

void createHorizontalWall(b2World &world, const float xpos, const float ypos, const float size){
    BasicObject* props = new BasicObject();
    props->color = cv::Scalar(128, 128, 128);
    for(float x = 0; x < size + 1; x += 2) {
        b2BodyDef bodyDef;
        bodyDef.position.Set(xpos + x, ypos);
        b2Body *groundBody = world.CreateBody(&bodyDef);
        groundBody->GetUserData().pointer = (uintptr_t) props;
        b2PolygonShape groundBox;
        groundBox.SetAsBox(1, 1);
        groundBody->CreateFixture(&groundBox, 0.0f);
    }
}

void createWallBox(b2World &world, const float size) {
    /*createVerticalWall(world, -1, -1, size);
    createHorizontalWall(world, -1, -1, size);
    createVerticalWall(world, size + 1, -1, size);
    createHorizontalWall(world, -1, size + 1, size);*/
}

World::World(float _worldHeight, float _worldWidth, float _timeStep)
    : distribution(0.0, 1.0), worldWidth(_worldWidth), worldHeight(_worldHeight), world(b2Vec2(0.0f, -0.0f)), timeStep(_timeStep)
{
    ContactListener *contactListenerInstance = new ContactListener();
    world.SetContactListener(contactListenerInstance);
    world.SetAllowSleeping(true);
    //createWallBox(world, worldSize);
}

b2Vec2 World::getRandomWorldPosition(){
    return( b2Vec2(distribution(generator) * worldWidth, distribution(generator) * worldHeight));
}

void World::addCreature(Creature *creature){
    this->activeObjects.push_back(creature);
}
void World::addPlant(Plant *plant){
    this->pasiveObjects.push_back(plant);
}
void World::step(){
    this->world.Step(this->timeStep, this->velocityIterations, this->positionIterations);
    this->world.ClearForces();
    std::vector<BasicObject *> survivingObjects;
    for(auto it: pasiveObjects){
        if(it->live()){
            survivingObjects.push_back(it);
        } else{
            delete it;
        }
    }
    pasiveObjects = survivingObjects;

    std::vector<Creature *> survivingCreatures;
    for(auto it: this->activeObjects){
        if(it->live()){
            survivingCreatures.push_back(it);
        } else {
            delete it;
        }
    }
    activeObjects = survivingCreatures;
}
//
// Created by ihradis on 3/1/21.
//

#ifndef DRIBLES_WORLD_H
#define DRIBLES_WORLD_H

#include "Creature.h"
#include <random>

class World {
    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution;
public:
    float worldWidth;
    float worldHeight;
    b2World world;
    std::vector<Creature *> activeObjects;
    std::vector<BasicObject *> pasiveObjects;
    const float timeStep = 1.0 / 10;
    const int velocityIterations = 2;
    const int positionIterations = 1;
    World(float _worldHeight, float _worldWidth, float _timeStep = 0.1);
    ~World(){
        for(int i = 0; i < activeObjects.size(); i++){
            delete activeObjects[i];
        }
        for(int i = 0; i < pasiveObjects.size(); i++){
            delete pasiveObjects[i];
        }
    }

    b2Vec2 getRandomWorldPosition();

    void step();

    void addCreature(Creature *);
    void addPlant(Plant *);
};


#endif //DRIBLES_WORLD_H

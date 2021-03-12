//
// Created by ihradis on 3/1/21.
//

#ifndef DRIBLES_CREATURE_H
#define DRIBLES_CREATURE_H

#include <iostream>
#include <vector>
#include <functional>
#include <box2d/box2d.h>
#include "VisionSensor.h"
#include <box2d/b2_world_callbacks.h>
#include "Actuator.h"

enum _entityCategory {
    BOUNDARY =          0x0001,
    CREATURE =     0x0002,
    PLANT =        0x0004,
    OTHER = 0x0008,
};

class Creature;

class BasicObject{
public:
    std::vector<BasicObject *> eatingThem;

    b2World* world = NULL;
    b2Body* body = NULL;

    std::vector<Sensor *> sensors;
    std::vector<Actuator *> actuators;

    float energy;
    float health;
    float age;
    cv::Scalar color;

    std::function<void(BasicObject *)> dieCallback;
    std::function<void(BasicObject *creature, std::vector<float> &percept)> perceptionCallback;
    std::function<std::vector<float>(BasicObject *)> getActionCallback;


    int getActuatorValueCount();
    float act();
    void sense();
    int getSenseSize();

    virtual bool live() {return true;};

    virtual ~BasicObject();
};


b2Body* createCircleObject(b2World &world, float px, float py, float angle, float radius, float density, float damping, float friction, float restitution, _entityCategory category);
void createMouthSensor(b2Body *body, float radius);


class Creature: public BasicObject {
public:
    float quality;

    float eatingScale = 1;
    float eatingTransferCoeff = 1;
    float energyLoss = 0.1;

    virtual ~Creature(){}

    Creature(b2World &world, float px, float py, float angle)
    {
        this->quality = 0;
        this->age = 0;
        this->color = cv::Scalar (0, 255,255);
        this->world = &world;
        this->energy = 100;
        this->health = 100;
        this->body = createCircleObject(world, px, py, angle,0.25, 1, 0.2, 0.2, 0.5, CREATURE);
        createMouthSensor(this->body, 0.25);
        this->body->GetUserData().pointer = (uintptr_t) this;

        VisionSensor * visionSensor = new VisionSensor(50, 0.4, 21);
        this->sensors.push_back(visionSensor);
        this->sensors.push_back(new MotionSensor());
        this->sensors.push_back(new HealthSensor());

        //this->actuators.push_back(new VisionActuator(visionSensor));
        this->actuators.push_back(new WheelActuator(0.25, 0.02, 0.05));
        this->actuators.push_back(new ColorActuator());
    }

    void addEating(BasicObject *obj){
        eatingThem.push_back(obj);
    }
    void removeEating(BasicObject *obj){
        eatingThem.erase( std::find(eatingThem.begin(), eatingThem.end(), obj ) );
    }
    virtual bool live(){
        this->age += 1.0;

        float eatingFraction = 0 * eatingScale;
        float eatingBite = 0.4 * eatingScale;
        for(auto &pray: eatingThem){
            float sourceEnergy = std::max(0.0f, pray->energy * eatingFraction + eatingBite);
            pray->energy -= sourceEnergy;
            pray->health -= 0;
            this->energy += eatingTransferCoeff * sourceEnergy;
        }

        this->energy -= this->energyLoss;
        this->energy = std::min(this->energy, 100.0f);
        this->health = std::min(this->health + (this->energy * 0.01f), 100.0f);

        const float a = 0.9995;
        this->quality = a * this->quality + (1.0f - a) * this->energy;

        return this->energy > 0 and this->health > 0;
    }
};

class Plant: public BasicObject {
    bool print;
public:
    virtual ~Plant()
    {}

    Plant(b2World &world, float px, float py, float radius, bool _print=false)
        :print(_print)
    {
        this->color =  cv::Scalar (255,128);
        this->world = &world;
        this->energy = 100;
        this->health = 100;
        this->body = createCircleObject(world, px, py, 0, radius, 2, 4.0, 0.5, 0.5, PLANT);
        this->body->GetUserData().pointer = (uintptr_t) this;
    }
    virtual bool live(){
        this->health -= 0.1;
        if(print) {
            std::cout << this->health << std::endl;
        }
        //this->energy = std::min(this->energy + 0.3, 100.0);;
        //this->health = std::min(this->health + (this->energy * 0.01), 100.0);
        this->color = cv::Scalar(this->energy / 100.0 * 255, 0, (100 - this->energy) / 100.0 * 255);
        return this->energy > 0 and this->health > 0;
    }
};

bool getCreatureAndPray(b2Contact* contact, Creature*& creature, BasicObject*& pray);

class ContactListener : public b2ContactListener
{
    void BeginContact(b2Contact* contact) {
        Creature* creature;
        BasicObject* pray;
        if ( getCreatureAndPray(contact, creature, pray) )
            creature->addEating(pray);
    }

    void EndContact(b2Contact* contact) {
        Creature* creature;
        BasicObject* pray;
        if ( getCreatureAndPray(contact, creature, pray) )
            creature->removeEating(pray);
    }
};

#endif //DRIBLES_CREATURE_H

//
// Created by ihradis on 3/1/21.
//

#ifndef DRIBLES_VISIONSENSOR_H
#define DRIBLES_VISIONSENSOR_H

#include <vector>
#include <box2d/box2d.h>
#include <opencv2/core.hpp>

#include "Actuator.h"

class Sensor{
public:
    virtual void sense(const b2World &world, b2Body* creatureBody, std::vector<float> &percept) = 0;
};


class VisionSensor: public Sensor{
public:
    std::vector<b2Vec2> visionVectors;
    float rotationAngle;
    VisionSensor(float distance = 25, float viewWidth = 1, int samples = 5);
    virtual ~VisionSensor(){};
    virtual void sense(const b2World &world, b2Body* creatureBody, std::vector<float> &percept);
};

class VisionActuator:  public Actuator{
    VisionSensor *sensor;
public:
    VisionActuator(VisionSensor *_sensor)
    : sensor(_sensor)
    {  }

    virtual int getValueCount() {return 2;}
    virtual float act(b2Body *body, std::vector<float>::iterator &actionIt) {
        const float x = *(actionIt++) - 0.5;
        const float y = *(actionIt++) - 0.5;
        sensor->rotationAngle = atan2f(x, y);
        return 0;
    }
};

class MotionSensor: public Sensor{
public:
    virtual ~MotionSensor(){};
    virtual void sense(const b2World &world, b2Body* creatureBody, std::vector<float> &percept);
};

class HealthSensor: public Sensor{
    float lastHealth;
    float lastEnergy;
public:
    virtual ~HealthSensor(){};
    virtual void sense(const b2World &world, b2Body* creatureBody, std::vector<float> &percept);
};

#endif //DRIBLES_VISIONSENSOR_H

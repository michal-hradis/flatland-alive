//
// Created by ihradis on 3/3/21.
//

#ifndef DRIBLES_ACTUATOR_H
#define DRIBLES_ACTUATOR_H

#include <vector>
#include <box2d/box2d.h>

class Actuator{
public:
    virtual int getValueCount() = 0;
    virtual float act(b2Body *body, std::vector<float>::iterator &actionIt) = 0;
};

class WheelActuator: public Actuator{
    float base;
    float forceScale;
    float energyScale;
public:
    WheelActuator(float _base, float _forceScale, float _energyScale):
            base(_base), forceScale(_forceScale), energyScale(_energyScale)
    {}
    virtual int getValueCount() {return 2;};
    virtual float act(b2Body *body, std::vector<float>::iterator &actionIt);
};

class ColorActuator: public Actuator{
public:
    ColorActuator()
    {}
    virtual int getValueCount() {return 3;};
    virtual float act(b2Body *body, std::vector<float>::iterator &actionIt);
};

#endif //DRIBLES_ACTUATOR_H

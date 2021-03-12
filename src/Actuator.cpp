//
// Created by ihradis on 3/3/21.
//

#include "Actuator.h"
#include "Creature.h"

//
float WheelActuator::act(b2Body *body, std::vector<float>::iterator &actionIt) {
    const float motionMean = 0.5;
    const float v1 = *(actionIt++) - motionMean;
    const float v2 = *(actionIt++) - motionMean;

    b2Vec2 direction = body->GetWorldVector( b2Vec2(0,1) );
    b2Vec2 baseVec = direction.Skew();
    baseVec *= this->base;
    b2Vec2 impulseRight = direction;
    b2Vec2 impulseLeft = direction;
    impulseLeft *= v1 * forceScale;
    impulseRight *= v2 * forceScale;

    body->ApplyLinearImpulse(impulseLeft, body->GetPosition() - baseVec, true);
    body->ApplyLinearImpulse(impulseRight, body->GetPosition() + baseVec, true);
    return (std::abs(v1) + std::abs(v2)) * this->energyScale;
};

float ColorActuator::act(b2Body *body, std::vector<float>::iterator &actionIt) {
    const int v1 = int(*(actionIt++) * 255);
    const int v2 = int(*(actionIt++) * 255);
    const int v3 = int(*(actionIt++) * 255);
    BasicObject* obj = (BasicObject*)body->GetUserData().pointer;
    obj->color = cv::Scalar(v1, v2, v3);
    return 0;
};
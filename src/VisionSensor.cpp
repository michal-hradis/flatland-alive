//
// Created by ihradis on 3/1/21.
//

#include "VisionSensor.h"
#include "Creature.h"

class VisionCallback: public b2RayCastCallback
{
public:
    b2Fixture* fixture = NULL;
    float distanceFraction = -1;
    int counter = 0;

    virtual ~VisionCallback() {}

    /// Called for each fixture found in the query. You control how the ray cast
    /// proceeds by returning a float:
    /// return -1: ignore this fixture and continue
    /// return 0: terminate the ray cast
    /// return fraction: clip the ray to this point
    /// return 1: don't clip the ray and continue
    /// @param fixture the fixture hit by the ray
    /// @param point the point of initial intersection
    /// @param normal the normal vector at the point of intersection
    /// @param fraction the fraction along the ray at the point of intersection
    /// @return -1 to filter, 0 to terminate, fraction to clip the ray for
    /// closest hit, 1 to continue
    virtual float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
                                const b2Vec2& normal, float fraction){
        counter += 1;
        this->fixture = fixture;
        this->distanceFraction = fraction;
        return fraction;
    };
};


VisionSensor::VisionSensor(float distance, float viewWidth, int samples)
    : rotationAngle(0.0f)
{
    for (float x = -viewWidth; x <= viewWidth + 0.001; x += 2*viewWidth / (samples-1)) {
        b2Vec2 v(x, 1);
        v.Normalize();
        v *= distance / (1 + abs(2*x));
        this->visionVectors.push_back(v);
    }
}
#include <iostream>

void VisionSensor::sense(const b2World &world, b2Body* creatureBody, std::vector<float> &percept) {
    const b2Rot rot(rotationAngle);
    const b2Vec2 basePoint = creatureBody->GetWorldPoint(b2Mul(rot, b2Vec2(0, 0.1)));
    VisionCallback callback;

    percept.push_back(sin(rotationAngle));
    percept.push_back(cos(rotationAngle));

    for(int i = 0; i < this->visionVectors.size(); i++){
        callback.counter = 0;
        world.RayCast(&callback, basePoint, creatureBody->GetWorldPoint(b2Mul(rot, this->visionVectors[i])));
        if(callback.counter) {
            const cv::Scalar &s = ((BasicObject *)callback.fixture->GetBody()->GetUserData().pointer)->color;
            percept.push_back(s.val[0] / 255.0);
            percept.push_back(s.val[1] / 255.0);
            percept.push_back(s.val[2] / 255.0);
            percept.push_back(1.0 - callback.distanceFraction);
        } else{
            percept.push_back(0.0);
            percept.push_back(0.0);
            percept.push_back(0.0);
            percept.push_back(0.0);
        }
    }
}

void MotionSensor::sense(const b2World &world, b2Body* body, std::vector<float> &percept){
    const b2Vec2 &v = body->GetLinearVelocity();
    percept.push_back(v.x);
    percept.push_back(v.y);
    percept.push_back(body->GetAngularVelocity());
}

void HealthSensor::sense(const b2World &world, b2Body* body, std::vector<float> &percept){
    Creature* c = reinterpret_cast<Creature*>(body->GetUserData().pointer);
    percept.push_back(c->energy / 100.0);
    percept.push_back(c->health / 100.0);
    percept.push_back((c->energy - lastEnergy) / 100.0);
    percept.push_back((c->health - lastHealth) / 100.0);
    lastEnergy = c->energy;
    lastHealth = c->health;
}
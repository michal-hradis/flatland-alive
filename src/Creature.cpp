//
// Created by ihradis on 3/1/21.
//

#include "Creature.h"



b2Body* createCircleObject(b2World &world, float px, float py, float angle, float radius, float density, float damping, float friction, float restitution, _entityCategory category) {
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.linearDamping = damping;
    bodyDef.angularDamping = damping * 0.3;
    bodyDef.position.Set(px, py);
    bodyDef.angle = angle;

    b2Body *body = world.CreateBody(&bodyDef);

    b2CircleShape circle;
    circle.m_p.Set(.0f, 0.0f);
    circle.m_radius = radius;

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circle;
    fixtureDef.density = density;
    fixtureDef.friction = friction;
    fixtureDef.restitution = restitution;
    fixtureDef.filter.categoryBits = category;
    body->CreateFixture(&fixtureDef);

    return body;
}

void createMouthSensor(b2Body *body, float radius) {
    b2CircleShape mouthCircle;
    mouthCircle.m_p.Set(.0f, radius);
    mouthCircle.m_radius = radius * 0.2;

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &mouthCircle;
    fixtureDef.isSensor = true;
    fixtureDef.filter.maskBits = PLANT | CREATURE;
    fixtureDef.filter.categoryBits = OTHER;
    body->CreateFixture(&fixtureDef);
}

bool getCreatureAndPray(b2Contact* contact, Creature*& creature, BasicObject*& pray)
{
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();

    //make sure only one of the fixtures was a sensor
    bool sensorA = fixtureA->IsSensor();
    bool sensorB = fixtureB->IsSensor();
    if ( ! (sensorA ^ sensorB) )
        return false;

    if ( sensorA ) { //fixtureB must be an enemy aircraft
        creature = reinterpret_cast<Creature*>(fixtureA->GetBody()->GetUserData().pointer);
        pray = reinterpret_cast<BasicObject*>(fixtureB->GetBody()->GetUserData().pointer);
    }
    else { //fixtureA must be an enemy aircraft
        creature = reinterpret_cast<Creature*>(fixtureB->GetBody()->GetUserData().pointer);
        pray = reinterpret_cast<BasicObject*>(fixtureA->GetBody()->GetUserData().pointer);
    }

    return true;
}

int BasicObject::getActuatorValueCount() {
    int count = 0;
    for(int i = 0; i < actuators.size(); i++) {
        count += actuators[i]->getValueCount();
    }
    return count;
}

float BasicObject::act(){
    std::vector<float> actions = this->getActionCallback(this);
    float burnedEnergy = 0;
    std::vector<float>::iterator actionIt = actions.begin();
    for(int i = 0; i < actuators.size(); i++) {
        burnedEnergy += actuators[i]->act(this->body, actionIt);
    }
    return burnedEnergy;
}

void BasicObject::sense(){
    std::vector<float> percept;
    percept.push_back(1.0f);
    for(int i = 0; i < sensors.size(); i++) {
        sensors[i]->sense(*world, body, percept);
    }
    this->perceptionCallback(this, percept);
}

int BasicObject::getSenseSize(){
    int size = 0;
    std::vector<float> percept;
    percept.push_back(1.0f);
    for(int i = 0; i < sensors.size(); i++) {
        sensors[i]->sense(*world, body, percept);
    }
    return percept.size();
}

BasicObject::~BasicObject(){
    if(dieCallback) {
        dieCallback(this);
    }

    if(this->body != NULL){
        this->world->DestroyBody(this->body);
    }
    for(int i = 0; i < this->sensors.size(); i++){
        delete this->sensors[i];
    }
    for(int i = 0; i < this->actuators.size(); i++){
        delete this->actuators[i];
    }
}



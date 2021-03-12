#include <iostream>
#include <fstream>

#include <vector>
#include <map>
#include <chrono>
#include <random>
#include <algorithm>
#include <functional>
#include <omp.h>
#include <box2d/box2d.h>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "World.h"
#include "Viewer.h"
#include "SimpleNN.h"
#include <mutex>

class BrainCollection{
    std::mutex mtx;
public:
    std::map<BasicObject *, SimpleNN *> brains;    std::map<BasicObject *, std::vector<float> > lastPercept;
    int hiddenLayerSize;
    float mutateStrength;
    float mutateProb;
    float mutateClearProb;

    BrainCollection(int _hiddenLayerSize, float _mutateStrength=0.1, float _mutateProb = 0.02, float _mutateClearProb = 0.02)
        : hiddenLayerSize(_hiddenLayerSize),
        mutateStrength( _mutateStrength), mutateProb( _mutateProb), mutateClearProb( _mutateClearProb)
    {}

    void addCallbacks(Creature *newCreature){
        newCreature->dieCallback = std::bind(&BrainCollection::dieCallback, this, std::placeholders::_1);
        newCreature->perceptionCallback = std::bind(&BrainCollection::perceptionCallback, this, std::placeholders::_1, std::placeholders::_2);
        newCreature->getActionCallback = std::bind(&BrainCollection::getActionCallback, this, std::placeholders::_1);
    }

    void addBrain(Creature *newCreature){
        addCallbacks(newCreature);
        brains[newCreature] = new SimpleNN(newCreature->getSenseSize(), newCreature->getActuatorValueCount(), hiddenLayerSize);
    }

    void addBrain(Creature *newCreature, Creature *parent1){
        addCallbacks(newCreature);
        brains[newCreature] = new SimpleNN(*brains[parent1]);
        brains[newCreature]->mutate(mutateStrength, mutateProb, mutateClearProb);
    }

    void addBrain(Creature *newCreature, Creature *parent1, Creature *parent2){
        addCallbacks(newCreature);
        brains[newCreature] = new SimpleNN(*brains[parent1]);
        brains[newCreature]->merge(brains[parent2]);
        brains[newCreature]->mutate(0.2, 0.02, 0.01);
    }

    void dieCallback(BasicObject *creature){
        mtx.lock();
        delete brains[creature];
        brains.erase(creature);
        lastPercept.erase(creature);
        mtx.unlock();
    }

    void perceptionCallback(BasicObject *creature, std::vector<float> &percept){
        if(lastPercept.count(creature)) {
            lastPercept[creature] = percept;
        } else {
            mtx.lock();
            lastPercept[creature] = percept;
            mtx.unlock();
        }
    }

    std::vector<float> getActionCallback(BasicObject *creature){
        return brains[creature]->step( lastPercept[creature]);
    }
};

int main() {

    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(0.0,1.0);
    //std::normal_distribution<double> distribution(0.0, 0.7);

    float worldScale = 2;
    double worldArea = 150 * 150 * worldScale * 2;
    float worldHeight = 50;
    float worldWidth = worldArea / worldHeight;
    int hiddenLayerSize = 64;
    float timeStep = 0.3;
    int viewStep = 6;
    float minPlantSize = 0.1;
    float maxPlantSize = 0.3;


    ViewerCV viewer(1200, 800, worldWidth, worldHeight, "wiever", true, false);
    //EmptyViewer viewer(1200, 800, worldWidth, "wiever");
    int activeCount = int(500 * worldScale);
    int pasiveCount = int(1100 * worldScale);
    std::cout << pasiveCount << ' ' << hiddenLayerSize << std::endl;

    World world(worldHeight, worldWidth, timeStep);
    BrainCollection brainCollection(48, 0.2, 0.025, 0.01);

    for(int i = 0; i < activeCount; i++) {
        b2Vec2 p = world.getRandomWorldPosition();
        Creature *newCreature = new Creature(world.world, p.x, p.y, distribution(generator) * 2 * 3.14);
        world.addCreature(newCreature);
    }

    {
        cv::FileStorage file("brains.yml", cv::FileStorage::READ);
        int storedBrainCount = 0;
        file["brain_count"] >> storedBrainCount;
        if (storedBrainCount > 0) {
            int counter = 0;
            for (int i = 0; i < world.activeObjects.size(); i++) {
                SimpleNN *net = new SimpleNN(world.activeObjects[i]->getSenseSize(),
                                             world.activeObjects[i]->getActuatorValueCount(), hiddenLayerSize);
                std::string net_name = "net_" + std::to_string(counter++ % storedBrainCount);
                net->load(file, net_name);
                brainCollection.brains[world.activeObjects[i]] = net;
                brainCollection.addCallbacks(world.activeObjects[i]);

            }
        } else {
            for (auto &creature: world.activeObjects) {
                brainCollection.addBrain(creature);
            }
        }
    }

    for(int i = 0; i < pasiveCount; i++){
        b2Vec2 p = world.getRandomWorldPosition();
        world.addPlant(new Plant(world.world, p.x, p.y, minPlantSize + distribution(generator) * (maxPlantSize - minPlantSize)));
    }


    for(int i = 0; i < 4000000; ++i) {
        auto start_time = std::chrono::high_resolution_clock::now();

        #pragma omp parallel for
        for (auto it = world.activeObjects.begin(); it < world.activeObjects.end(); it++) {
            auto &creature = *it;
            creature->sense();
        }
        world.step();
        #pragma omp parallel for
        for (auto it = world.activeObjects.begin(); it < world.activeObjects.end(); it++) {
            auto &creature = *it;
            creature->energy -= creature->act() * 5;
        }
        while(world.pasiveObjects.size() < pasiveCount){
            b2Vec2 p = world.getRandomWorldPosition();
            world.addPlant(new Plant(world.world, p.x, p.y, minPlantSize + distribution(generator) * (maxPlantSize - minPlantSize)));
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto time = end_time - start_time;

        if(i % viewStep == 0) {
            //std::cout << " Took " << time/std::chrono::milliseconds(1) << "ms to run. " << world.activeObjects.size() << std::endl;
            //std::cout << world.world.GetTreeHeight() << ' ' << world.world.GetTreeBalance() << ' ' << world.world.GetTreeQuality() << std::endl;
            viewer.renderWorld(&world, i);
            int key = cv::waitKey(1);
            switch(key){
                case 'a':
                    viewStep = int((viewStep + 1) * 1.1);
                    std::cout << viewStep << std::endl;
                    break;
                case 's':
                    viewStep = std::max(1, int((viewStep - 1) / 1.1));
                    std::cout << viewStep << std::endl;
                    break;
                case 'f':
                case '-':
                    viewer.zoom(1/1.2, viewer.imgWidth / 2, viewer.imgHeight / 2);
                    break;
                case 'g':
                case '+':
                    viewer.zoom(1.2, viewer.imgWidth / 2, viewer.imgHeight / 2);
                    break;
                case 'l': {
                    cv::FileStorage file("brains.yml", cv::FileStorage::WRITE);
                    file << "brain_count" << (int) brainCollection.brains.size();
                    int counter = 0;
                    for (auto net: brainCollection.brains) {
                        net.second->save(file, "net_" + std::to_string(counter++));
                    }
                    std::cout << "STORE" << std::endl;
                    break;
                }
                case 27:
                    exit(0);
                    break;
            }
        }

        if(i > 0 and i % 10000 == 0){
            cv::FileStorage file(f"brains_{i:06d}.yml", cv::FileStorage::WRITE);
            file << "brain_count" << (int) brainCollection.brains.size();
            int counter = 0;
            for (auto net: brainCollection.brains) {
                net.second->save(file, "net_" + std::to_string(counter++));
            }
            std::cout << "STORE" << std::endl;
        }

        if(i % 250 == 0 or (world.activeObjects.size() < activeCount * 0.9 and i % 10 == 0)) {
            std::vector<Creature *> creatures = world.activeObjects;
            double averageAge = 0;
            std::sort(creatures.begin(), creatures.end(), [](auto &a, auto &b) { return a->quality > b->quality;});
            for(int j = 0; j < 100 and j < creatures.size(); j++ ) {
                averageAge += creatures[j]->age;
            }
            averageAge /= std::min((int)creatures.size(), 100);

            if(averageAge > 5000){
                pasiveCount = int(pasiveCount * 0.995);
            }

            int toGenerate = std::min(activeCount - world.activeObjects.size(), creatures.size());
            for(int j = 0; j < toGenerate; j++ ) {
                b2Vec2 p = creatures[j]->body->GetWorldCenter();
                Creature *newCreature = new Creature(world.world, p.x - 1, p.y, distribution(generator) * 2 * 3.14);

                if(j > toGenerate * 0.95) {
                    brainCollection.addBrain(newCreature);
                } else if(distribution(generator) < 0.7){
                    brainCollection.addBrain(newCreature, creatures[j]);
                } else {
                    brainCollection.addBrain(newCreature, creatures[j], creatures[(j + 1) % creatures.size()] );
                }
                world.addCreature(newCreature);
            }

            //std::cout << "Took " << i << ' ' << time/std::chrono::milliseconds(1) << "ms to run. " << world.world.GetTreeHeight() << ' ' << world.world.GetTreeBalance() << ' ' << world.world.GetTreeQuality() << std::endl;
            std::cout << "MUTATE " << i << ' ' <<  time/std::chrono::milliseconds(1) << ' ' << pasiveCount << ' ' << creatures.size() << ' ' << world.activeObjects.size() << ' ' << averageAge << ' ' << creatures[0]->age << ' ' << creatures[0]->energy << std::endl;
            std::vector<double> energy(20);
            for(auto &creature: creatures){
                for(int i = 0; i < brainCollection.brains[creature]->W.size(); i++) {
                    cv::Mat tmp;
                    cv::pow(brainCollection.brains[creature]->W[i], 2, tmp);
                    energy[i] += sqrt(cv::sum(tmp)[0]);
                }
            }
        }
    }

    return 0;
}

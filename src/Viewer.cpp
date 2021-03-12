//
// Created by ihradis on 03/03/2021.
//

#include "Viewer.h"
#include <iostream>

cv::Point ViewerCV::pointFromWorld(const b2Vec2 &p ){
    int x = int((p.x - viewportCenterX) / pixelSize + imgWidth/2.0 + 0.5);
    int y = int((p.y - viewportCenterY) / pixelSize + imgHeight/2.0 + 0.5);
    return cv::Point(x, y);
}

void ViewerCV::zoom(float factor, float x, float y){
    pixelSize *= factor;
}

void getVisionVector(const VisionSensor &sensor, b2Body* creatureBody, b2Vec2 &p1, std::vector<b2Vec2> &p2){
    const b2Rot rot(sensor.rotationAngle);
    p1 = creatureBody->GetWorldPoint(b2Mul(rot, b2Vec2(0, 0.1)));
    for(auto &vec: sensor.visionVectors) {
        b2Vec2 visionDir = creatureBody->GetWorldPoint(b2Mul(rot, vec)) - p1;
        visionDir.Normalize();
        p2.push_back(p1 + visionDir);
    }
}

void ViewerCV::renderWorld(World *world, int iteration){
    if(lastFrameStart - iteration > 10000){
        lastFrameStart = iteration
        videoWriter.close();
        std::ostringstream ss;
        ss << std::setw(6) << std::setfill('0') << lastFrameStart;
        std::string str = ss.str();
        videoWriter("out_" + str +" .avi", CV_FOURCC('M','J','P','G'), 30, cv::Size(imgWidth, imgHeight)
    }

    screenImage = cv::Scalar::all(0);
    for (Creature *creature: world->activeObjects) {
        cv::Point p = this->pointFromWorld(creature->body->GetPosition());
        if(p.x > 0 and p.y > 0 and p.x < imgWidth and p.y < imgHeight) {
            cv::Point p2 = this->pointFromWorld(creature->body->GetWorldPoint(b2Vec2(0, 1)));
            int size = std::max(1, int(0.25 / pixelSize + 0.5));
            cv::circle(screenImage, p, size, creature->color);
            cv::line(screenImage, p, p2, creature->color);
            if(size > 1) {
                if (creature->age < 255) {
                    cv::circle(screenImage, p, size * 2, cv::Scalar::all(255 - creature->age));
                }

                b2Vec2 v1;
                std::vector<b2Vec2> v2All;
                getVisionVector(*(VisionSensor *) creature->sensors[0], creature->body, v1, v2All);
                for (b2Vec2 &v2: v2All) {
                    cv::line(screenImage, this->pointFromWorld(v1), this->pointFromWorld(v2), cv::Scalar::all(255));
                }
            }
        }
    }

    for (int i = 0; i < world->pasiveObjects.size(); i++) {
        cv::Point p = this->pointFromWorld(world->pasiveObjects[i]->body->GetPosition());
        int size = std::max( 1, int(world->pasiveObjects[i]->body->GetFixtureList()->GetShape()->m_radius / pixelSize + 0.5));
        cv::circle(screenImage, p, size, world->pasiveObjects[i]->color);
    }
    if(renderScreen) {
        cv::imshow(windowName, screenImage);
    }
    if(renderVideo){
        videoWriter.write(screenImage);
    }
}


void ViewerCV_mouseCallback(int event, int x, int y, int flags, void* userdata){
    ViewerCV *viewer = (ViewerCV *) userdata;
    switch(event) {
        case cv::EVENT_MBUTTONDOWN:
            viewer->lastMouseX = x;
            viewer->lastMouseY = y;
            viewer->mouseDrag = true;
            break;
        case cv::EVENT_MBUTTONUP:
            viewer->mouseDrag = false;
            break;
        case cv::EVENT_MOUSEMOVE:
            if(viewer->mouseDrag){
                viewer->viewportCenterX -= (x - viewer->lastMouseX) * viewer->pixelSize;
                viewer->viewportCenterY -= (y - viewer->lastMouseY) * viewer->pixelSize;
                viewer->lastMouseX = x;
                viewer->lastMouseY = y;
            }
            break;
        case cv::EVENT_MOUSEHWHEEL:
            if (cv::getMouseWheelDelta(flags) > 0)
                viewer->zoom(1.2, x, y);
            else
                viewer->zoom(1 / 1.2, x, y);
            break;
    }
}
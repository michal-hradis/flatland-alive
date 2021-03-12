//
// Created by ihradis on 03/03/2021.
//

#ifndef DRIBLES_VIEWER_H
#define DRIBLES_VIEWER_H

#include <memory>
#include <box2d/box2d.h>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "World.h"


void ViewerCV_mouseCallback(int event, int x, int y, int flags, void* userdata);

class EmptyViewer{
public:
    int imgWidth;
    int imgHeight;
    EmptyViewer(int _imgWidth, int _imgHeight, float worldSize, std::string _windowName="wiever"){}
    void zoom(float factor, float x, float y){}
    void renderWorld(World *world){}
};

class ViewerCV{
public:
    int imgWidth;
    int imgHeight;
    float viewportCenterX;
    float viewportCenterY;
    float pixelSize;
    std::string windowName;
    cv::Mat screenImage;

    float lastMouseX;
    float lastMouseY;
    bool mouseDrag;
    bool renderVideo;
    bool renderScreen;
    cv::VideoWriter videoWriter;
    int lastFrameStart;

    ViewerCV(int _imgWidth, int _imgHeight, float worldWidth, float worldHeight, std::string _windowName="wiever",
             bool _renderVideo=false, bool _renderScreen=true)
            : imgWidth(_imgWidth), imgHeight(_imgHeight),
              viewportCenterX(worldWidth / 2), viewportCenterY(worldHeight / 2),
              windowName(_windowName), screenImage(_imgHeight, _imgWidth,CV_8UC3),
              renderVideo(_renderVideo), renderScreen(_renderScreen),
              videoWriter("out_000000.avi", CV_FOURCC('M','J','P','G'), 30, cv::Size(imgWidth, imgHeight)),
              lastFrameStart(0)
    {
        this->pixelSize = worldHeight / std::min(imgWidth, imgHeight);
        if(renderScreen) {
            cv::namedWindow(windowName);
            cv::setMouseCallback(windowName, ViewerCV_mouseCallback, this);
        }
    }

    cv::Point pointFromWorld(const b2Vec2 &p );

    void zoom(float factor, float x, float y);

    void renderWorld(World *world, int iteration);
};


#endif //DRIBLES_VIEWER_H

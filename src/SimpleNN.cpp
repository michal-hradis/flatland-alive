//
// Created by ihradis on 3/2/21.
//

#include <random>
#include "SimpleNN.h"

cv::Mat sigmoid(const cv::Mat &m){
    cv::Mat data = m.clone();
    cv::exp(-data, data);
    data = 1.0 /(1 + data);
    return data;
}

std::default_random_engine mutationGenerator;
std::normal_distribution<double> distribution(0.0, 1.0);

void mutateW(cv::Mat &w, float strength, float mutateProb, float clearProb){
    int mutationCount = std::binomial_distribution(w.rows*w.cols, mutateProb)(mutationGenerator);
    int clearCount = std::binomial_distribution(w.rows*w.cols, clearProb)(mutationGenerator);
    std::binomial_distribution(w.rows*w.cols, clearProb)(mutationGenerator);
    for(int i = 0; i < mutationCount; i++){
        w.at<float>(rand() % w.rows, rand() % w.cols) += distribution(mutationGenerator) * strength;
    }
    for(int i = 0; i < clearCount; i++){
        w.at<float>(rand() % w.rows, rand() % w.cols) = 0;
    }
}
//
// Created by ihradis on 3/2/21.
//

#ifndef DRIBLES_SIMPLENN_H
#define DRIBLES_SIMPLENN_H
#include <vector>
#include <opencv2/core.hpp>
#include <iostream>
#include <cstdlib>

cv::Mat sigmoid(const cv::Mat &m);
void mutateW(cv::Mat &w, float strength, float mutateProb, float clearProb);

inline cv::MatExpr softStep(cv::Mat &m){
    return( cv::min(1, cv::max(0, m)));
}
inline cv::MatExpr softStep(cv::MatExpr m){
    return( cv::min(1, cv::max(0, m)));
}

class LinearLayer{
public:
    cv::Mat W;
    cv::Mat b;
    LinearLayer(int inputSize, int featureCount)
        : W(inputSize, featureCount, CV_32F), b(1, hiddenSize, CV_32F))
    {
        initW();
    }
    void initW(){
        cv::randn(W, 0, sqrt(1.0/W.cols));
        b = 0;
    }
    auto operator()(cv::Mat &x) {
        return x * W + b;
    }
    auto operator()(cv::MatExpr &x) {
        return x * W + b;
    }
};


class HyperNN {

};



class SimpleNN {
public:
    std::vector<cv::Mat> W;
    std::vector<cv::Mat> b;
    const int w1 = 0;
    const int wOut = 1;
    const int storeW = 2;
    const int readW = 3;
    const int w2 = 4;

    cv::Mat memory;
    SimpleNN(int inputSize, int outputSize, int hiddenSize=16)
        : memory(1, hiddenSize, CV_32F)
    {
        W.push_back(cv::Mat(inputSize, hiddenSize, CV_32F));
        W.push_back(cv::Mat(hiddenSize, outputSize, CV_32F));
        W.push_back(cv::Mat(hiddenSize, hiddenSize, CV_32F));
        W.push_back(cv::Mat(hiddenSize, hiddenSize, CV_32F));
        W.push_back(cv::Mat(hiddenSize, hiddenSize, CV_32F));
        b.push_back(cv::Mat(1, hiddenSize, CV_32F));
        b.push_back(cv::Mat(1, outputSize, CV_32F));
        b.push_back(cv::Mat(1, hiddenSize, CV_32F));
        b.push_back(cv::Mat(1, hiddenSize, CV_32F));
        b.push_back(cv::Mat(1, hiddenSize, CV_32F));
        this->initW();
    }

    SimpleNN(const SimpleNN &old)
        : memory(old.memory.clone())
    {
        for(const auto &mW: old.W){
            W.push_back(mW.clone());
        }
        for(auto &mb: old.b){
            b.push_back(mb.clone());
        }
    }

    void merge(SimpleNN *secondNN){
        for(int i = 0; i < this->W.size(); i ++){
            cv::Mat randM(W[i].clone());
            cv::randu(randM, cv::Scalar(0), cv::Scalar(1));
            randM = (randM > 0.25) / 255;
            randM.convertTo(randM, CV_32F);
            this->W[i] = randM.mul(this->W[i]) + (1-randM).mul(secondNN->W[i]);
        }
        for(int i = 0; i < this->b.size(); i ++){
            cv::Mat randM(b[i].clone());
            cv::randu(randM, cv::Scalar(0), cv::Scalar(1));
            randM = (randM > 0.25) / 255;
            randM.convertTo(randM, CV_32F);
            this->b[i] = randM.mul(this->b[i]) + (1-randM).mul(secondNN->b[i]);
        }
    }

    void load(cv::FileStorage &file, std::string &name){
        for(int i = 0; i < this->W.size(); i ++){
            file[name + "_W_" + std::to_string(i)] >> this->W[i];
        }
        for(int i = 0; i < this->W.size(); i ++){
            file[name + "_b_" + std::to_string(i)] >> this->b[i];
        }
        file[name + "_memory"] >> this->memory;
    }

    void save(cv::FileStorage &file, std::string name){
        for(int i = 0; i < this->W.size(); i ++){
            file << name + "_W_" + std::to_string(i) << this->W[i];
        }
        for(int i = 0; i < this->W.size(); i ++){
            file << name + "_b_" + std::to_string(i) << this->b[i];
        }
        file << name + "_memory" << this->memory;
    }

    void initW(){
        float scale = 0.4;
        for(auto &m: this->W){
            cv::randn(m, 0, scale);
        }
        for(auto &m: this->b){
            m = 0;
        }
        memory = 0;
    }

    std::vector<float> step(std::vector<float> &inputV){
        cv::Mat input(1, inputV.size(), CV_32F, inputV.data());

        cv::Mat hidden = softStep(input * W[w1] + b[w1]);

        cv::Mat readGate = softStep((hidden + memory) * W[readW] + b[readW]- 5);
        hidden = readGate.mul(memory) + (1-readGate).mul(hidden);

        hidden = softStep(hidden * W[w2] + b[w2]);

        cv::Mat writeGate = softStep(hidden * W[storeW] + b[storeW]);
        memory = writeGate.mul(memory) + (1-writeGate).mul(hidden);

        cv::Mat output = sigmoid(hidden * W[wOut] + b[wOut]);

        std::vector<float> outputV(output.cols);
        for(int i = 0; i < output.cols; i++){
            outputV[i] = output.at<float>(0, i);
        }
        return outputV;
    }

    void mutate(float strength=0.1, float mutateProb = 0.02, float clearProb = 0.02){
        for(auto &mW: W){
            mutateW(mW, strength, mutateProb, clearProb);
        }
        for(auto &mb: b){
            mutateW(mb, strength, mutateProb, clearProb);
        }
    }
};


class HyperNN {

};

/*
class SimpleNN {
public:
    cv::Mat w1;
    cv::Mat wOut;
    cv::Mat w2;
    SimpleNN(int inputSize, int outputSize, int hiddenSize=16)
            : w1(inputSize, hiddenSize, CV_32F), w2(hiddenSize, hiddenSize, CV_32F), wOut(hiddenSize, outputSize, CV_32F)
    {
        this->initW();
    }

    SimpleNN(const SimpleNN &old)
            : w1(old.w1.clone()), w2(old.w2.clone()), wOut(old.wOut.clone())
    {
    }

    void initW(){
        cv::randu(w1, -0.3, 0.3);
        cv::randu(w2, -0.3, 0.3);
        cv::randu(wOut, -0.3, 0.3);
    }

    std::vector<float> step(std::vector<float> &inputV){
        cv::Mat input(1, inputV.size(), CV_32F, inputV.data());
        input = input * w1;
        input = cv::min(cv::max(input, 0), 1) * w2;
        input = cv::min(cv::max(input, 0), 1) * wOut;
        //cv::exp(input, input);
        input = sigmoid(input);//1.0 /(1 + input);

        std::vector<float> outputV(input.cols);
        for(int i = 0; i < input.cols; i++){
            outputV[i] = input.at<float>(0, i);
        }
        return outputV;
    }

    void mutate(float strength=0.1, int count = 5){
        for(int i = 0; i < count; i++){
            w1.at<float>(rand() % w1.rows, rand() % w1.cols) += ((rand() % 100) - 50.0) / 100.0 * strength;
        }
        for(int i = 0; i < count; i++){
            w2.at<float>(rand() % w2.rows, rand() % w2.cols) += ((rand() % 100) - 50.0) / 100.0 * strength;
        }
        for(int i = 0; i < count; i++){
            wOut.at<float>(rand() % wOut.rows, rand() % wOut.cols) += ((rand() % 100) - 50.0) / 100.0 * strength;
        }
    }
};*/

#endif //DRIBLES_SIMPLENN_H

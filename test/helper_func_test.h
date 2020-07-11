//
// Created by thang on 10/07/2020.
//

#include <iostream>
#include "../include/helper.h"
using namespace std;
const float delta = 0.001;

template <class T>
void assert(bool expression, std::string message, T have, T want){
    if(!expression) {
        std::cout << "Test failed: " << message << " have " << have << " want " << want << '\n';
        exit(0);
    }
}

void max_entropy_test (){
    const vector<int> labels = {1, 0, 1, 1, 0, 0};
    int label = 2;
    unordered_set<int> index_set;
    for(int i = 0; i < labels.size(); i++){
        index_set.insert(i);
    }

    float entropy = get_entropy(labels, label, index_set);
    float correct_entropy = 1;
    assert(abs(correct_entropy - entropy) < delta, "Entropy value incorrect", entropy, correct_entropy);
}

void min_entropy_test(){
    const vector<int> labels = {1, 1, 1, 1};
    int label = 2;
    unordered_set<int> index_set;
    for(int i = 0; i < labels.size(); i++){
        index_set.insert(i);
    }

    float entropy = get_entropy(labels, label, index_set);
    float correct_entropy = 0;
    assert(abs(correct_entropy - entropy) < delta, "Entropy value incorrect", entropy, correct_entropy);
}

void random_entropy_test(){
    const vector<int> labels = {1, 0, 1, 1, 1, 0, 0};
    int label = 2;
    unordered_set<int> index_set;
    for(int i = 0; i < labels.size(); i++){
        index_set.insert(i);
    }

    float entropy = get_entropy(labels, label, index_set);
    float correct_entropy = 0.98522;
    assert(abs(correct_entropy - entropy) < delta, "Entropy value incorrect", entropy, correct_entropy);
}

void least_split_info_1(){
    float sf = get_split_info(5,5);
    assert<float>(sf == 0.0f, "Wrong split info" ,sf, 0.0f);
}

void least_split_info_2(){
    float sf = get_split_info(0,5);
    assert<float>(sf == 0.0f, "Wrong split info" ,sf, 0.0f);
}

void max_split_info(){
    float sf = get_split_info(2, 4);
    assert<float>(sf == 0.5f, "Wrong split info" ,sf, 0.5f);
}

void majority_label_test(){
    std::vector<int> labels = {1,0,1,2,2,1,0,2,2};
    std::unordered_set<int> index_set = {0,1,2,3,4,5,6,7,8};

    int majority_label = get_majority_label(labels, index_set, 3);
    assert<int>(majority_label == 2, "Wrong threshold", majority_label, 2);
}

void threshold_test(){
    std::vector<float> unique_sorted = {1,3,5,7,9};

    std::vector<float>thresholds = get_thresholds(unique_sorted);
    assert<float>(thresholds[0] == -4.0f, "Wrong threshold", thresholds[0], -4);
    assert<float>(thresholds[1] == 2.0f, "Wrong threshold", thresholds[1], 2.0f);
    assert<float>(thresholds[2] == 4.0f, "Wrong threshold", thresholds[2], 4.0f);
    assert<float>(thresholds[3] == 6.0f, "Wrong threshold", thresholds[3], 6.0f);
    assert<float>(thresholds[4] == 8.0f, "Wrong threshold", thresholds[4], 8.0f);
    assert<float>(thresholds[5] == 14.0f, "Wrong threshold", thresholds[5], 14.0f);
}

void split_info_tests(){
    least_split_info_1();
    least_split_info_2();
    max_split_info();
}


void entropy_tests(){
    max_entropy_test();
    min_entropy_test();
    random_entropy_test();
}
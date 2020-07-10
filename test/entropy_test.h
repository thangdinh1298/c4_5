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


void entropy_tests(){
    max_entropy_test();
    min_entropy_test();
    random_entropy_test();
}
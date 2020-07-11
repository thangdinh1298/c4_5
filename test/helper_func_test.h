//
// Created by thang on 10/07/2020.
//

#include <iostream>
#include "../include/helper.h"
using namespace std;
const float delta = 0.001;

template <class T>
void Assert(bool expression, string message, T have, T want){
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
    Assert<float>(abs(correct_entropy - entropy) < delta, "Entropy value incorrect", entropy, correct_entropy);
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
    Assert<float>(abs(correct_entropy - entropy) < delta, "Entropy value incorrect", entropy, correct_entropy);
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
    Assert<float>(abs(correct_entropy - entropy) < delta, "Entropy value incorrect", entropy, correct_entropy);
}

void least_split_info_1(){
    float sf = get_split_info(5,5);
    Assert<float>(sf == 0.0f, "Wrong split info" ,sf, 0.0f);
}

void least_split_info_2(){
    float sf = get_split_info(0,5);
    Assert<float>(sf == 0.0f, "Wrong split info" ,sf, 0.0f);
}

void max_split_info(){
    float sf = get_split_info(2, 4);
    Assert<float>(sf == 0.5f, "Wrong split info" ,sf, 0.5f);
}

void majority_label_test(){
    std::vector<int> labels = {1,0,1,2,2,1,0,2,2};
    std::unordered_set<int> index_set = {0,1,2,3,4,5,6,7,8};

    int majority_label = get_majority_label(labels, index_set, 3);
    Assert<int>(majority_label == 2, "Wrong threshold", majority_label, 2);
}

void get_best_att_tests(){
    int label_num = 2;
    vector<int> labels = {1, 1, 0, 0};
    vector<int> col_category_count = {0, 3};
    vector<Attribute::Type> col_types = {Attribute::CONTINUOUS, Attribute::CATEGORICAL};
    vector<string> col_names = {"ca", "thal"};
    unordered_set<int> index_set = {0,1,2,3};
    unordered_set<int> col_set = {0, 1};
    rapidcsv::Document doc("../test/temp.csv");

    Attribute::attribute_t att = get_best_att(labels,col_category_count, col_types, col_names, index_set, col_set, doc);
    Assert<int>(att.att_index_ == 0, "Wrong best att", att.att_index_, 0);
    std::cout<<att.threshold_<<'\n';
}

void optimal_threshold_tests(){
    unordered_set<int> index_set = {0,1,2,3,4,5};
    vector<float> col = {1.0f, 3.0f, 5.0f, 7.0f, 10.0f, 20.0f};
    vector<int> labels = {0, 1, 1, 1, 1, 1};
    int label_num = 2;

    auto pair = get_optimal_threshold(label_num, get_entropy(labels, label_num, index_set), index_set, col, labels);
    cout<<pair.second<<'\n';
    Assert<float>(pair.first == 2.0f, "Wrong optimal threshold", pair.first, 2.0f);

    labels = {0, 1, 0, 1, 1, 1}; //entropy 0.97
    pair = get_optimal_threshold(label_num, get_entropy(labels, label_num, index_set), index_set, col, labels);
    Assert<float>(pair.first == 2.0f, "Wrong optimal threshold", pair.first, 2.0f);

}

void threshold_test(){
    std::vector<float> unique_sorted = {1,3,5,7,9};

    std::vector<float>thresholds = get_thresholds(unique_sorted);
    Assert<float>(thresholds[0] == -4.0f, "Wrong threshold", thresholds[0], -4);
    Assert<float>(thresholds[1] == 2.0f, "Wrong threshold", thresholds[1], 2.0f);
    Assert<float>(thresholds[2] == 4.0f, "Wrong threshold", thresholds[2], 4.0f);
    Assert<float>(thresholds[3] == 6.0f, "Wrong threshold", thresholds[3], 6.0f);
    Assert<float>(thresholds[4] == 8.0f, "Wrong threshold", thresholds[4], 8.0f);
    Assert<float>(thresholds[5] == 14.0f, "Wrong threshold", thresholds[5], 14.0f);
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
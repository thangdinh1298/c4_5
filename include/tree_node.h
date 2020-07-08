//
// Created by thang on 27/06/2020.
//

#ifndef C4_5_TREE_NODE_H
#define C4_5_TREE_NODE_H

#include <string>
#include <vector>
#include "attribute.h"

namespace TreeNode {
enum Type {
    LEAF = 0,
    CATEGORICAL_NODE = 1,
    CONTINUOUS_NODE = 2
};

class TreeNode {
public:
    TreeNode() = default;
    TreeNode(std::string att_name,
            int category_count, int index)
            : type_(CATEGORICAL_NODE), att_name_(att_name),
            children_(category_count, nullptr), att_index_(index)
    {
    }

    TreeNode(std::string att_name,
            float threshold, int index) : type_(CONTINUOUS_NODE), att_name_(att_name),
            threshold_(threshold), children_(2, nullptr), att_index_(index)
    {
    }

    TreeNode(int label) : type_(LEAF), label_(label){
    }

    void add_categorical(int category, TreeNode* node) {
        children_[category] = node;
    }

    void add_continuous(bool less, TreeNode* node){
        if (less) children_[0] = node;
        else children_[1] = node;
    }

    template<class T>
    TreeNode* test(T value){
        if(type_ == CONTINUOUS_NODE){
            return (value < threshold_) ? children_[0] : children_[1];
        } else if(type_ == CATEGORICAL_NODE) return children_[value];
        else return nullptr;
    }

    Type getType() const {
        return type_;
    }

    const std::string &getAttName() const {
        return att_name_;
    }

    int getAttIndex() const {
        return att_index_;
    }

    float getThreshold() const {
        return threshold_;
    }

    void print_node(){
        if(type_ == CONTINUOUS_NODE) {
            std::cout << "=============================\n";
            std::cout << "Continuous node for attribute " << att_name_ <<
            "with threshold " << threshold_<<'\n';
        } else if(type_ == CATEGORICAL_NODE){
            std::cout << "=============================\n";
            std::cout << "Categorical node for attribute " << att_name_ << '\n';
        } else {
            std::cout << "=============================\n";
            std::cout << "Leaf node with label " << label_ << '\n';
        }
    }

    const std::vector<TreeNode *> &getChildren() const {
        return children_;
    }

protected:
    Type type_;
    int label_;
    std::string att_name_;
    std::vector<TreeNode*> children_;
    int att_index_; //TODO: INITIALIZE THIS
    float threshold_;
};
}

#endif //C4_5_TREE_NODE_H

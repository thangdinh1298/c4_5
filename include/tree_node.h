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
        NON_LEAF = 0,
        LEAF = 1
    };

    class TreeNode {
    public:
        TreeNode() = default;
        TreeNode(std::string att_name,
                int category_count)
                : type_(NON_LEAF), att_name_(att_name),
                category_count_(category_count), att_type_(Attribute::CATEGORICAL)
        {
            children_.reserve(category_count);
        }

        TreeNode(std::string att_name,
                float threshold) : type_(NON_LEAF), att_name_(att_name),
                threshold_(threshold), att_type_(Attribute::CONTINUOUS)
        {
        }

        TreeNode(int label) : type_(LEAF), label_(label){
        }

        void add_child_node() {

        }

        TreeNode* get_next_node_categorical(int category){
            return children_[category];
        }

        TreeNode* get_next_node_continuous(float val){
            return (val < threshold_) ? children_[0] : children_[1];
        }

    protected:
        Type type_;
        int label_;
        std::string att_name_;
        Attribute::Type att_type_;
        int category_count_;
        std::unordered_map<TreeNode*> children_;
        float threshold_;
    };
}

#endif //C4_5_TREE_NODE_H

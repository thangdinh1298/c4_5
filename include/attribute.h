#include <utility>

//
// Created by thang on 26/06/2020.
//

#ifndef C4_5_ATTRIBUTE_H
#define C4_5_ATTRIBUTE_H

namespace Attribute {
enum Type : int {
    INVALID = 0,
    CATEGORICAL = 1,
    CONTINUOUS = 2,
    LABEL = 3
};

struct attribute_t {
    Type type_;
    float threshold_;
    std::string att_name_;
    int att_index_;

    attribute_t(Type t, std::string att_name="", int att_index=-1, float threshold=0) :
            type_(t), att_name_(std::move(att_name)), threshold_(threshold), att_index_(att_index)
    {

    }
};
}

#endif //C4_5_ATTRIBUTE_H

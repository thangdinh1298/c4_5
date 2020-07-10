//
// Created by thang on 09/07/2020.
//

#ifndef C4_5_STAT_H
#define C4_5_STAT_H

#include <iostream>

struct stat_t{
    unsigned correct = 0;
    unsigned wrong = 0;

    void print_stat(){
        std::cout<<"===================="<<'\n';
        std::cout<<"Statistics correct = "<<correct<<" wrong = "<<wrong<<'\n';
    }
};
#endif //C4_5_STAT_H

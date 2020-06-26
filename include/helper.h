//
// Created by thang on 26/06/2020.
//

#ifndef C4_5_HELPER_H
#define C4_5_HELPER_H

#include <vector>
#include <unordered_set>

float get_split_info(size_t k, size_t n){
    return (k == 0) ? 0 : -((static_cast<float>(k)/n) * log2((static_cast<float>(k)/n)));
}

std::vector<float> get_thresholds(const std::vector<float>& unique_sorted){
    std::vector<float> results;
    if (unique_sorted.empty()) return results;
    results.push_back(unique_sorted[0] - 5.);
    for(auto it = unique_sorted.begin(); it + 1 != unique_sorted.end(); it++){
        float thresh = (*it + *it+1)/2;
        results.push_back(thresh);
    }
    results.push_back(unique_sorted[unique_sorted.size() - 1] + 5.);

    return results;
}

template<class IndexCollection = std::unordered_set<int>>
float get_entropy(const std::vector<int>& label,
                  int label_num,
                  const IndexCollection& index_set){
    std::vector<int> category_count (label_num, 0);
    for(auto index: index_set){
        category_count[label[index]]++;
    }

    float entropy = 0;
    for(auto num: category_count){
        if (num == 0) continue;
        entropy += (static_cast<float>(num)/index_set.size()) *
                    log2(static_cast<float>(num)/index_set.size());
    }
    return -entropy;
}

#endif //C4_5_HELPER_H

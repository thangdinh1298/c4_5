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
        float thresh = (*it + *(it+1))/2;
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

bool is_pure(const std::vector<int>& labels,
             const std::unordered_set<int>& index_set)
{
    int first_label = labels[*index_set.begin()];
    for(auto it = index_set.begin(); it != index_set.end(); it++){
        if(labels[*it] != first_label) return false;
    }
    return true;
}

int get_majority_label(
                const std::vector<int> &labels,
                const std::unordered_set<int> &index_set,
                int label_num)
{
//    std::cout<<"Label num is: "<<label_num<<'\n';
    if(index_set.empty()) return -1;
    std::vector<int> labels_count(label_num, 0);
    for(auto index: index_set) labels_count[labels[index]]++;
    int max_label_count = -1;
    int max_label = -1;
    for(int i = 0; i < labels_count.size(); i++) {
        int label_count = labels_count[i];
        if (label_count > max_label_count) {
            max_label_count = label_count;
            max_label = i;
        }
    }
//    std::cout<<"Max label is: "<<max_label<<std::endl;
    return max_label;
}

#endif //C4_5_HELPER_H

//
// Created by thang on 26/06/2020.
//

#ifndef C4_5_HELPER_H
#define C4_5_HELPER_H

#include <vector>
#include <unordered_set>
#include <algorithm>
#include "attribute.h"
#include "rapid.h"

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

/*
 * This method determines the best threshold to split a continuous attribute (column)
 * into two parts
 */
std::pair<float, float> get_optimal_threshold(
        int label_num,
        float entropy_b4_split,
        const std::unordered_set<int>& index_set,
        const std::vector<float>& col,
        const std::vector<int>& labels)
{
    std::vector<float> attribute_val;
    attribute_val.reserve(index_set.size());
    for(auto index: index_set){
        attribute_val.push_back(col[index]);
    }

    //Sort indices vector by their attribute value
    std::sort(attribute_val.begin(), attribute_val.end(), [col](const auto& lhs, const auto& rhs){
        return lhs < rhs;
    });

    attribute_val.erase(std::unique(attribute_val.begin(), attribute_val.end()), attribute_val.end());
    std::vector<float> thresholds = get_thresholds(attribute_val);

    float optimal_threshold = 0.0;
    float optimal_gain_ratio = 0.0;
    for(auto threshold: thresholds){
        std::unordered_set<int> lesser_index_set, greater_index_set;
        for(auto index: index_set){
            if(col[index] < threshold) lesser_index_set.insert(index);
            else greater_index_set.insert(index);
        }
        float entropy_after_split = 0;
        entropy_after_split += get_entropy(labels, label_num, lesser_index_set) *
                               lesser_index_set.size() / index_set.size();
        entropy_after_split += get_entropy(labels, label_num, greater_index_set) *
                               greater_index_set.size() / index_set.size();

        float information_gain = entropy_b4_split - entropy_after_split;
        float split_info = get_split_info(lesser_index_set.size(), index_set.size())
                           + get_split_info(greater_index_set.size(), index_set.size());

        float gain_ratio = (information_gain == 0) ? 0 : information_gain/split_info;
        if (gain_ratio > optimal_gain_ratio){
            optimal_gain_ratio = gain_ratio;
            optimal_threshold = threshold;
        }
    }

    return std::make_pair(optimal_threshold, optimal_gain_ratio);
}

/*
 * This method split the index_set using the best column in the column_set
 * and builds a node for the corresponding split
 * this function assumes the index set passed in is impure
 */
Attribute::attribute_t get_best_att(const std::vector<int>& labels, // label for each row
                                    const std::vector<int>& col_category_count, // category count for each column
                                    const std::vector<Attribute::Type>& col_types, // types of each column
                                    const std::vector<std::string>& col_names, // name of each column
                                    const std::unordered_set<int>& index_set, //set of rows to split
                                    const std::unordered_set<int>& col_set, //set of column (attributes) to consider
                                    const rapidcsv::Document& doc)
{
    int label_num = col_category_count[col_names.size() - 1];
    float entropy_b4_split = get_entropy(labels, label_num, index_set);
    float optimal_gain_ratio = 0.0;
    float optimal_threshold = 0.0;
    int split_col_index = -1;

    for(auto i: col_set){
//        std::cout << "Considering column " << i << " " << col_names[i] << '\n';
        float gain_ratio = 0.0;
        float threshold = 0.0;
        if(col_types[i] == Attribute::CATEGORICAL){ //Always do a multi-way split on categorical attributes
            std::vector<int> col = doc.GetColumn<int>(col_names[i]);
            int k = col_category_count[i];
            std::vector<std::unordered_set<int>> category_index_set(k, std::unordered_set<int>());
            // Compute how many items belong to each category after split
            for(auto row: index_set){
                category_index_set[col[row]].insert(row);
            }
            float entropy_after_split = 0;
            for(int j = 0; j < k; j++){
                entropy_after_split += get_entropy(labels, label_num, category_index_set[j]) *
                                       category_index_set[j].size()/index_set.size();
            }
            float split_info = 0;
            for(int j = 0; j < k; j++){
                split_info += get_split_info(category_index_set[j].size(), index_set.size());
            }
            float information_gain = entropy_b4_split - entropy_after_split;
            gain_ratio = (information_gain == 0) ? 0 : information_gain/split_info;


        } else if (col_types[i] == Attribute::CONTINUOUS){ //Always do a binary split on continuous attributes
            std::vector<float> col = doc.GetColumn<float>(col_names[i]);
            std::pair<float, float> result = get_optimal_threshold(label_num, entropy_b4_split, index_set, col, labels);
            threshold = result.first;
            gain_ratio = result.second;
        } else { //LABEL type, do nothing
            continue;
        }
        if (gain_ratio > optimal_gain_ratio) {
//            std::cout<<"Index "<<i<<" has gain ratio "<<gain_ratio <<" greater than "<<optimal_gain_ratio<<'\n';
            optimal_gain_ratio = gain_ratio;
            split_col_index = i;
            optimal_threshold = threshold;
        }

//        std::cout << "For column " << col_names[i] << " gain ratio would be " << gain_ratio << '\n';
    }

    if(split_col_index != -1) {
//        std::cout << "Split by " << col_names[split_col_index] << " with gain ratio " << optimal_gain_ratio;
//        if (col_types[split_col_index] == Attribute::CONTINUOUS) std::cout << " with threshold " << optimal_threshold;
//        std::cout << '\n';
    } else {
//        std::cout << "No valid index was found" << '\n';
    }

    if(split_col_index == -1){
        return Attribute::attribute_t(Attribute::INVALID);
    }
    if(col_types[split_col_index] == Attribute::CONTINUOUS){
        return Attribute::attribute_t(Attribute::CONTINUOUS,
                                      col_names[split_col_index], split_col_index, optimal_threshold);
    }
    if(col_types[split_col_index] == Attribute::CATEGORICAL){
        return Attribute::attribute_t(Attribute::CATEGORICAL, col_names[split_col_index], split_col_index);
    }
}

#endif //C4_5_HELPER_H

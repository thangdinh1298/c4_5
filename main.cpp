#include <iostream>
#include <math.h>
#include <fstream>
#include "include/rapid.h"
#include "include/attribute.h"
#include <algorithm>
#include <climits>
#include <unordered_set>

float get_entropy(const std::vector<int>& label){
    size_t positive = 0, negative = 0, total = label.size();
    for(int i = 0; i < label.size(); i++){
        if(label[i] == 0) negative++;
        else positive++;
    }
    return (positive == 0 || negative == 0) ? 0 :
            -(static_cast<float>(negative)/total * log2(static_cast<float>(negative)/total)
            + static_cast<float>(positive)/total * log2(static_cast<float>(positive)/total));
}

float get_entropy(const std::vector<int>& label,
                  int label_num,
                  const std::unordered_set<int>& index_set){
    std::vector<int> category_count (label_num, 0);
    for(auto index: index_set){
        category_count[label[index]]++;
    }

    float entropy = 0;
    for(auto num: category_count){
        if (num == 0) continue;
        entropy += static_cast<float>(num)/index_set.size();
    }
    return -entropy;
}

//int get_split_point(const std::vector<std::pair<float, int>>& col){
//    float min_entropy = INT_MAX;
//    int split_point = -1, i = 0;
//    for(auto it = col.begin(); it < col.end(); it++, i++){
//        float entropy = get_entropy(std::vector<std::pair<float, int>>(col.begin(), it))
//                + get_entropy(std::vector<std::pair<float, int>>(it, col.end()));
//        if(min_entropy > entropy)
//        {
//            min_entropy = entropy;
//            split_point = i;
//        }
//    }
//}

/*
 * This method split the index_set using the best column in the column_set
 * and builds a node for the corresponding split
 */
void recurse_build_tree(const std::vector<int>& labels, // label for each row
                        const std::vector<int>& col_category_count, // category count for each column
                        const std::vector<Attribute::Type>& col_types, // types of each column
                        const std::vector<std::string>& col_names, // name of each column
                        std::unordered_set<int> index_set, //set of rows to split
                        std::unordered_set<int> col_set, //set of column (attributes) to consider
                        const rapidcsv::Document& doc)
{
    int label_num = col_category_count[col_names.size() - 1];
    float entropy_b4_split = get_entropy(labels, label_num, index_set);
    for(auto i: col_set){
        if(col_types[i] == Attribute::CATEGORICAL){ //Always do a multi-way split on categorical attributes
            std::vector<int> col = doc.GetColumn<int>(col_names[i]);
            int k = col_category_count[i];
//            std::vector<int> category_item_count(k, 0);
            std::vector<std::unordered_set<int>> category_index_set(k, std::unordered_set<int>());
            // Compute how many items belong to each category after split
            for(auto row: index_set){
//                category_item_count[col[row]]++;
                category_index_set[col[row]].insert(row);
            }
            float entropy_after_split = 0;
            for(int j = 0; j < k; j++){
                entropy_after_split += get_entropy(labels, label_num, category_index_set[j]);
            }
            float split_info = 0;
            for(int j = 0; j < k; j++){
                split_info += -(static_cast<float>(category_index_set[j].size())/index_set.size());
            }

            float gain_ratio = (entropy_b4_split - entropy_after_split)/split_info;

        } else if (col_types[i] == Attribute::CONTINUOUS){ //Always do a binary split on continuous attributes

        } else { //LABEL type, do nothing
            continue;
        }
    }
}

int main(int argc, char** argv) {
    if(argc != 3){
        std::cout << "Usage: program [csv_data_file] [column_desc_file]" << '\n';
        return 1;
    }
    std::string data_file_name = argv[1];
    std::string col_desc_file = argv[2];

    rapidcsv::Document doc(data_file_name);
    std::vector<std::string> column_names;
    std::vector<Attribute::Type> column_types;
    std::vector<int> column_category_count;

    std::ifstream desc_f(col_desc_file);
    if(!desc_f){
        std::cout << "Error opening [column_desc_file]" << '\n';
        return 1;
    }

    std::string type_s, class_num_s;
    while(desc_f >> type_s >> class_num_s){
        try{
            int type = std::stoi(type_s);
            int class_num = std::stoi(class_num_s);
            column_types.push_back(static_cast<Attribute::Type>(type));
            column_category_count.push_back((type == Attribute::CATEGORICAL
                                            || type == Attribute::LABEL)
                                            ? class_num : 0);
        } catch (std::invalid_argument& e){
            std::cout << e.what() << '\n';
            return 1;
        }
    }

    if(column_types.size() != doc.GetColumnCount()){
        std::cout << "Column count discrepancy between 2 files" << '\n';
        return 1;
    }

    int label_col_idx = -1;
    for(size_t i = 0; i < doc.GetColumnCount(); i++){
        column_names.push_back(doc.GetColumnName(i));
        if(column_types[i] == Attribute::LABEL) {
            if (label_col_idx == -1) label_col_idx = i;
            else {
                std::cout << "There can only be one label column" << '\n';
                return 1;
            }
        }
        std::cout<<"Column name: "<< column_names[i]<<"\t Column type: "
                                  << column_types[i]<< "\t Column category count: "
                                  << column_category_count[i] << '\n';
    }

    if(label_col_idx == -1){
        std::cout << "No label column was found" << '\n';
        return 1;
    }
    std::vector<int> labels = doc.GetColumn<int>(label_col_idx);



    //For every attribute, determine the normalized IG
    //Choose best Attribute
    //Recurse

//    std::vector<int> label = doc.GetColumn<int>("target");
//    int i = 10;
//    while (i > 0){
//        i--;
//        float original_entropy = get_entropy(label);
//        for(auto col_name: cols){
//            auto col = doc.GetColumn<float>(col_name);
//            auto col_with_label = std::vector<std::pair<float, int>>();
//            for(int i = 0; i < label.size(); i++){
//                col_with_label.push_back(std::make_pair(col[i], label[i]));
//            }
//            std::sort(col_with_label.begin(), col_with_label.end(), [](auto const& l, auto const& r){
//                return l.first < r.first;
//            });
//
////            for(auto p: col_with_label){
////                std::cout<<p.first<<": "<<p.second<<"\n";
////            }
//        }
//    }
}

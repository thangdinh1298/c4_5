#include <iostream>
#include <math.h>
#include "include/helper.h"
#include <fstream>
#include "include/rapid.h"
#include "include/attribute.h"
#include "include/tree_node.h"
#include "include/stat.h"
#include <algorithm>
#include <unordered_set>


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
        std::cout << "Considering column " << i << " " << col_names[i] << '\n';
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
            optimal_gain_ratio = gain_ratio;
            split_col_index = i;
            optimal_threshold = threshold;
        }

        std::cout << "For column " << col_names[i] << " gain ratio would be " << gain_ratio << '\n';
    }

    if(split_col_index != -1) {
        std::cout << "Split by " << col_names[split_col_index] << " with gain ratio " << optimal_gain_ratio;
        if (col_types[split_col_index] == Attribute::CONTINUOUS) std::cout << " with threshold " << optimal_threshold;
        std::cout << '\n';
    } else {
        std::cout << "No valid index was found" << '\n';
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
/*
 * This method assumes index set is not empty
 */
TreeNode::TreeNode* build_tree(
        int label_num,
        const std::vector<int>& labels, // label for each row
        const std::vector<int>& col_category_count, // category count for each column
        const std::vector<Attribute::Type>& col_types, // types of each column
        const std::vector<std::string>& col_names, // name of each column
        const std::unordered_set<int>& index_set, //set of rows to split
        const std::unordered_set<int>& col_set, //set of column (attributes) to consider
        const rapidcsv::Document& doc)
{
//    if (index_set.empty() || col_set.empty()) return nullptr;
    /*else */if(is_pure(labels, index_set)) {
        return new TreeNode::TreeNode (labels[*index_set.begin()]);
    }
    // get attribute
    Attribute::attribute_t attribute = get_best_att(labels, col_category_count,
                                        col_types, col_names, index_set, col_set, doc);

    TreeNode::TreeNode* node = nullptr;

    //divide the index set into subsets and build tree from that subset
    switch (attribute.type_) {
        case Attribute::CATEGORICAL : {
            int column_id = attribute.att_index_;
            std::vector<int> col = doc.GetColumn<int>(column_id);
            int category_count = col_category_count[column_id];
            std::vector<std::unordered_set<int>> subsets(category_count, std::unordered_set<int>());

            //build subsets
            for (auto index: index_set) {
                subsets[col.at(index)].insert(index);
            }

            int non_empty_set_count = std::count_if(subsets.begin(), subsets.end(),
                                      [](std::unordered_set<int> s){
                                            return !s.empty();
                                      });
            if (non_empty_set_count <= 1) { //If everything gets directed into the same set, stop the tree
                return new TreeNode::TreeNode(get_majority_label(labels, index_set, label_num));
            }

            std::unordered_set<int> subset_column(col_set.begin(), col_set.end());
            subset_column.erase(column_id);

            if(subset_column.empty()) { // if no more column to split, return majority
                return new TreeNode::TreeNode(get_majority_label(labels, index_set, label_num));
            }

            node = new TreeNode::TreeNode(attribute.att_name_,
                    col_category_count[attribute.att_index_], attribute.att_index_);

            for (int i = 0; i < subsets.size(); i++) {
                std::unordered_set<int> subset = subsets[i];
                //if child node has no record to split on, just return a leaf node with majority from parent
                TreeNode::TreeNode *child = (subset.empty()) ?
                                            new TreeNode::TreeNode(get_majority_label(labels, index_set, label_num)) :
                                                build_tree(label_num, labels, col_category_count, col_types, col_names,
                                                       subset, subset_column, doc);

                node->add_categorical(i, child); //Attach the child node to the parent node
            }
            break;
        }
        case Attribute::CONTINUOUS: {
            int column_id = attribute.att_index_;
            std::vector<int> col = doc.GetColumn<int>(column_id);

            std::vector<std::unordered_set<int>> subsets(2, std::unordered_set<int>());
            for (auto index: index_set) {
                if(col.at(index) < attribute.threshold_) {
                    subsets[0].insert(index);
                } else subsets[1].insert(index);
            }

            if(subsets[0].empty() || subsets[1].empty()) {
                return new TreeNode::TreeNode(get_majority_label(labels, index_set, label_num));
            }

            std::unordered_set<int> subset_column(col_set.begin(), col_set.end());
            subset_column.erase(column_id);

            if(subset_column.empty()) { // if no more column to split, return majority
                return new TreeNode::TreeNode(get_majority_label(labels, index_set, label_num));
            }

            node = new TreeNode::TreeNode(attribute.att_name_,
                                        attribute.threshold_, attribute.att_index_);

            TreeNode::TreeNode* lesser_node = build_tree(label_num, labels, col_category_count, col_types, col_names,
                                  subsets[0], subset_column, doc);
            TreeNode::TreeNode* greater_node = build_tree(label_num, labels, col_category_count, col_types, col_names,
                                       subsets[1], subset_column, doc);

            node->add_continuous(true, lesser_node);
            node->add_continuous(false, greater_node);
            break;
        }
        case Attribute::INVALID:{
            return new TreeNode::TreeNode(get_majority_label(labels, index_set, label_num));
            break;
        }
        default:
            break;
    }
    //return the node
    return node;
}

void print_tree(TreeNode::TreeNode* root, int level=0){
    if(root == nullptr) {
        std::cout << "Undefined node" << '\n';
        return;
    }
    std::cout<<"Level "<<level<<'\n';
    root->print_node(1);
    auto children = root->getChildren();
    for(auto child: children) {
        print_tree(child, level+1);
    }
}

bool verify_row(TreeNode::TreeNode* root, const std::vector<int>& labels, int index, const rapidcsv::Document& doc){
    if (root->getType() == TreeNode::LEAF){
        return labels[index] == root->getLabel();
    } else if(root->getType() == TreeNode::CATEGORICAL_NODE){
        int row_category = doc.GetCell<int>(root->getAttIndex(), index);
        return verify_row(root->getChildren()[row_category], labels, index, doc);
    } else {
        int row_value = doc.GetCell<float>(root->getAttIndex(), index);
        return (row_value < root->getThreshold()) ?
                    verify_row(root->getChildren()[0], labels, index, doc):
                    verify_row(root->getChildren()[1], labels, index, doc);
    }
}

stat_t verify_set(  TreeNode::TreeNode* root, //root of the C4.5 tree
                    const std::vector<int>& labels,
                    const std::unordered_set<int>& index_set, //set of rows to split
                    const rapidcsv::Document& doc
                    )
{
    stat_t stat;
    for (auto index: index_set){
        bool result = verify_row(root, labels, index, doc);
        if(result) stat.correct++;
        else stat.wrong++;
    }
    return stat;
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

    std::unordered_set<int> test_set;
    std::unordered_set<int> index_set;
    size_t test_set_size = 50;
    for(int i = 0; i < doc.GetRowCount() - test_set_size; i++) index_set.insert(i);
    for(int i = doc.GetRowCount() - test_set_size; i < doc.GetRowCount(); i++) test_set.insert(i);

    std::unordered_set<int> column_set;
    for(int i = 0; i < doc.GetColumnCount(); i++) column_set.insert(i);
//    get_tree_node(
//        labels,
//        column_category_count,
//        column_types,
//        column_names,
//        index_set,
//        column_set,
//        doc
//    );

    TreeNode::TreeNode* root = build_tree(column_category_count[doc.GetColumnCount() - 1],
            labels, column_category_count, column_types, column_names, index_set, column_set, doc);
    print_tree(root);
    stat_t stat = verify_set(root, labels, test_set, doc);
    stat.print_stat();
}

#include <iostream>
#include "include/helper.h"
#include <fstream>
#include "test/helper_func_test.h"
#include "include/attribute.h"
#include "include/tree_node.h"
#include "include/stat.h"
#include <algorithm>
#include <unordered_set>



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
                                      [](const std::unordered_set<int>& s){
                                            return !s.empty();
                                      });
            if (non_empty_set_count <= 1) { //If everything gets partitioned into the same set, return a leaf
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
//        std::cout << "Label of record is " << labels[index] << " leaf node label is " << root->getLabel() << '\n';
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
    /*
     * Run tests
     */
    entropy_tests();
    split_info_tests();
    threshold_test();
    majority_label_test();
    optimal_threshold_tests();
    get_best_att_tests();

    /*
     * Run program
     */
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
    std::unordered_set<int> column_set;
    for(int i = 0; i < doc.GetColumnCount(); i++) column_set.insert(i);
    size_t k = 5; //number of partitions for cross validation

    //Cross validation
    {
        size_t partition_size = doc.GetRowCount() / k;
        size_t remainder = doc.GetRowCount() % k;

        std::vector<std::vector<int>> partitions(k, std::vector<int>());
        for(int i = 0; i < doc.GetRowCount(); i++){
            size_t partition_index = i / partition_size;
            if (partition_index >= k) partition_index--;
            partitions[partition_index].push_back(i);
        }

        for(int i = 0; i < k; i++){
            std::unordered_set<int> test_set;
            std::unordered_set<int> index_set;
            test_set.insert(partitions[i].begin(), partitions[i].end());
            for(int j = 0; j < i; j++){
                index_set.insert(partitions[j].begin(), partitions[j].end());
            }
            for(int j = i+1; j < k; j++){
                index_set.insert(partitions[j].begin(), partitions[j].end());
            }

            TreeNode::TreeNode* root = build_tree(column_category_count[doc.GetColumnCount() - 1],
                                                  labels, column_category_count, column_types, column_names, index_set, column_set, doc);

//            print_tree(root);
            stat_t stat = verify_set(root, labels, test_set, doc);
            stat.print_stat();
        }
    }
}

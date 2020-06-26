# Input: 
- csv_data_file: csv file with data
- col_desc_file: This file describe the information 
about each column in the data file. Each line 
describe the corresponding column index with 2 
numbers: type of column (either CATEGORICAL = 1, CONTINUOUS = 2
or LABEL = 3) and number of categories for the column
(0 if the column type is CONTINUOUS)
# Ouput: 
- Decision tree

# Note
The Label column has to be the last column in the csv
and col_desc file.
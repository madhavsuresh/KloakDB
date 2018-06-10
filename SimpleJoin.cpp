#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <iostream>
using namespace std;

vector< vector<int> > simplejoin(vector< vector<int> > table1,
                                 vector< vector<int> > table2, int join_column1_id, int join_column2_id){
    // table1 double for loop
    vector<int> join_column1 = table1[join_column1_id];
    vector<int> join_column2 = table2[join_column2_id];

    // got rid of the join column
    table1.erase(table1.begin() + join_column1_id);
    table2.erase(table2.begin() + join_column2_id);
    vector< vector<int> > other_table1 = table1;
    vector< vector<int> > other_table2 = table2;

    vector<int> common;

    vector< vector<int> > common_table1;
    vector< vector<int> > common_table2;
    vector<int> common_table1_ind;
    vector<int> common_table2_ind;

    vector< vector<int> > result;
    int join_size = join_column1.size();

    if(join_size != join_column2.size())
    {
        throw "join columns are not the same size";
    }

    // get all the common indexes
    // use a set of indexes
    for(int i = 0; i < join_size; i++)
    {
        for(int j = 0; j < join_size; j++)
        {
            if(join_column1[i] == join_column2[j])
            {
                common.push_back(join_column1[j]);
                common_table1_ind.push_back(i);
                common_table2_ind.push_back(j);
            }
        }
    }
    /*

    unordered_map<string, > hashmap;
    // iterate through the first table, getting the hash value of the join column as the key and the rest as the value.
    for (int i = 0; i < join_size; i++)
    {
        hashmap.insert(to_string(join_column1[i]));
    }
    */



    // for loop for getting inner table1
    for(int j = 0; j < other_table1.size(); j++)
    {

        vector<int> column;
        for(int i = 0; i < common_table1_ind.size(); i++)
        {
            column.push_back(other_table1[j][common_table1_ind[i]]);
        }
        common_table1.push_back(column);

    }

    // for loop for getting inner table2
    for(int j = 0; j < other_table2.size(); j++)
    {
        vector<int> column;
        for(int i = 0; i < common_table2_ind.size(); i++)
        {
            column.push_back(other_table2[j][i]);
        }
        common_table2.push_back(column);
    }

    // push_back all three to the result and return;
    result.insert( result.begin(), common_table2.begin(), common_table2.end());
    result.insert( result.begin(), common_table1.begin(), common_table1.end());
    result.push_back(common);
    return result;
}

vector< vector< vector<int> > > hashjoin(vector< vector<int> > table1,
                                 vector< vector<int> > table2, int join_column1_id, int join_column2_id){
    // table1 double for loop
    vector<int> int_join_column1 = table1[join_column1_id];
    vector<int> int_join_column2 = table2[join_column2_id];

    // got rid of the join column
    table1.erase(table1.begin() + join_column1_id);
    table2.erase(table2.begin() + join_column2_id);
    vector< vector<int> > other_table1 = table1;
    vector< vector<int> > other_table2 = table2;

    vector<int> common;

    vector< vector< vector<int> > > result;
    int join_size = int_join_column1.size();

    if(join_size != int_join_column2.size())
    {
        throw "join columns are not the same size";
    }
    if(join_size == 0 || int_join_column2.size() == 0)
    {
        throw "join column can't be 0 of length";
    }
    vector<string> str_pred_1;
    vector<string> str_pred_2;
    for(int i = 0; i < join_size; i++)
    {
        str_pred_1.push_back(to_string(int_join_column1[i]));
        str_pred_2.push_back(to_string(int_join_column2[i]));
    }

    unordered_map<string, vector<int> > hashmap;
    map<string, vector<int> > hashmap2;
    // iterate through the first table, getting the hash value of the join column as the key and the rest as the value.
    for (int i = 0; i < join_size; i++)
    {
        unordered_map<string, vector<int> >::iterator iter = hashmap.find(str_pred_1[i]);
        // check if it's already there, (non primary key join)
        if(iter == hashmap.end())
        {
            // did not find, so must add.
            vector<int> value;
            value.push_back(other_table1[0][i]);
            hashmap.insert( {str_pred_1[i], value} );
        } else 
        {
            // found, so the value must be updated
            iter->second.push_back(other_table1[0][i]);
        }
    }

    // make a map for the second table.
    for (int i = 0; i < join_size; i++)
    {    
        map<string, vector<int> >::iterator iter = hashmap2.find(str_pred_2[i]);
        // check if it's already there, (non primary key join)
        if(iter == hashmap2.end())
        {
            // did not find, so must add.
            vector<int> value;
            value.push_back(other_table2[0][i]);
            hashmap2.insert( {str_pred_2[i], value} );
        } else 
        {
            // found, so the value must be updated
            iter->second.push_back(other_table2[0][i]);
        }
    }
    // Iterate through the map of the second table's keys, then merge with the first one.
    map<string, vector<int> >::iterator iter;
    vector< vector<int> > key;
    vector< vector<int> > first;
    vector< vector<int> > second;
    for (iter = hashmap2.begin(); iter != hashmap2.end(); ++iter)
    {
        unordered_map<string, vector<int> >::iterator iter1 = hashmap.find(iter->first);
        if (iter1 != hashmap.end())
        {
            // found a common element
            key.push_back(vector<int>(1, stoi(iter->first)));
            first.push_back(iter1->second);
            second.push_back(iter->second);
        }
    }
    result.push_back(key);
    result.push_back(first);
    result.push_back(second);

    return result;
}

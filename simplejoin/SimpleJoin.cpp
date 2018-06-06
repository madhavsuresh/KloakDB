#include "SimpleJoin.h"

#include <vector>
#include <set>
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
    set<int> common_index;

    // now common_index has the value
    /*
    for (int i = 0; i < join_size; i++)
    {
        common_index.insert(join_column1[i]);
        common_index.insert(join_column2[i]);
    }
    */



    // for loop for getting inner table1
    for(int i = 0; i < common_table1_ind.size(); i++)
    {

        vector<int> row;
        for(int j = 0; j < other_table1.size(); j++)
        {
            row.push_back(other_table1[j][i]);
        }
        common_table1.push_back(row);
    }


    // for loop for getting inner table2
    for(int i = 0; i < common_table2_ind.size(); i++)
    {

        vector<int> row;
        for(int j = 0; j < other_table2.size(); j++)
        {
            row.push_back(other_table2[j][i]);
        }
        common_table2.push_back(row);
    }

    // push_back all three to the result and return;
    result.insert( result.begin(), common_table2.begin(), common_table2.end());
    result.insert( result.begin(), common_table1.begin(), common_table1.end());
    result.push_back(common);
    return result;
}

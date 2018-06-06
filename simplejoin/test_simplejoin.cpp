#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdlib>

#include <SimpleJoin.h>
#include <debug/DebugInSGXController.h>
#include <debug/DebugFakeLocalDataMover.h>
#include <vector>

#include "kAnonymousOperatorTestFixture.h"
using namespace std;
TEST_F(KAnonymousOperatorTestFixture, KAnonymousUnionParseAndRun) {
    // make 2 tables, 2d vector
    vector<int> other_column1;
    vector<int> other_column2;
    vector<int> common_column1;
    vector<int> common_column2;

    for(int i = 0 ; i < 20; i++)
    {
        other_column1.push_back(i*(rand()% 50));
        other_column2.push_back(i*(rand()% 50));
        common_column1.push_back(i);
        common_column2.push_back(i+5);
    }
    // 1
    vector< vector<int> > table1;
    table1.push_back(other_column1);
    table1.push_back(common_column1);
    // 2
    vector< vector<int> > table2;
    table2.push_back(common_column2);
    table2.push_back(other_column2);

    // input: 2 tables with the join column specified for both tables
    //      vector< vector<int> > table 1, vector< vector<int> > table2, join columnid1, join columnid2
    // output: 1 table that is joined
    //      vector< vector<int> >
    vector< vector<int> > joined_table = SimpleJoin.simplejoin(table1, table2, 1, 0);

    // assert check that the output length is as expected
    ASSERT_EQ(3, joined_table.length);
    ASSERT_EQ(15, joined_table[0].length);
    ASSERT_EQ(15, joined_table[1].length);
    ASSERT_EQ(15, joined_table[2].length);

    // expected table
    // assert check that the output content is as expected

}




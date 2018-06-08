#include <gtest/gtest.h>
#include <cstdlib>

#include "SimpleJoin.h"
#include <vector>

namespace
{
    TEST(Join_Length_Test, Join)
    {
        // make 2 tables, 2d vector
        std::vector<int> other_column1;
        std::vector<int> other_column2;
        std::vector<int> common_column1;
        std::vector<int> common_column2;

        for(int i = 0 ; i < 20; i++)
        {
            other_column1.push_back(i*(rand()% 50));
            other_column2.push_back(i*(rand()% 50));
            common_column1.push_back(i);
            common_column2.push_back(i+5);
        }
        // 1
        std::vector< vector<int> > table1;
        table1.push_back(other_column1);
        table1.push_back(common_column1);
        // 2
        std::vector< vector<int> > table2;
        table2.push_back(common_column2);
        table2.push_back(other_column2);

        // input: 2 tables with the join column specified for both tables
        // vector< vector<int> > table 1, vector< vector<int> > table2, join columnid1, join columnid2
        // output: 1 table that is joined
        // vector< vector<int> >
        std::vector< std::vector<int> > joined_table = simplejoin(table1, table2, 1, 0);

        // assert check that the output length is as expected
        ASSERT_EQ(3, joined_table.size());
        ASSERT_EQ(15, joined_table[0].size());
        ASSERT_EQ(15, joined_table[1].size());
        ASSERT_EQ(15, joined_table[2].size());

        // expected table
        // assert check that the output content is as expected

    }
}

int main(int ac, char* av[])
{
    ::testing::InitGoogleTest(&ac, av);
    return RUN_ALL_TESTS();
}




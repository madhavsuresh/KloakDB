//
// Created by madhav on 7/27/18.
//

#include "postgres_client_test.h"
#include "postgres_client.h"

class postgres_client_test : public ::testing::Test {
public:
    pqxx::result t_random_500;
    pqxx::result res2;
protected:
    void SetUp() override {
        // This test assumes query works... also existence of "t_random_500"
        // This test can just create the database we want
        std::string query_string("SELECT * FROM t_random_500");
        std::string dbname("dbname=test");
        t_random_500 = query(query_string, dbname);
    }

    void TearDown() override {

    };

};


TEST_F(postgres_client_test, get_schema) {
    table_t t;
    schema_t *s = get_schema_from_query(t_random_500, &t);
    ASSERT_EQ(s->num_fields, 2);
    ASSERT_STRCASEEQ(s->fields[0].field_name, "s");
    ASSERT_EQ(s->fields[0].col_no, 0);
    ASSERT_EQ(s->fields[0].type, INT);
    ASSERT_STRCASEEQ(s->fields[1].field_name, "floor");
    ASSERT_EQ(s->fields[1].col_no, 1);
    ASSERT_EQ(s->fields[1].type, INT);
    ASSERT_LE(malloc_usable_size(s), 328);
    ASSERT_EQ(t.schema.num_fields, s->num_fields);
}
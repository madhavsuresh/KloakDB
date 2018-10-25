//
// Created by root on 10/4/18.
//
#include "HashJoin.h"
#include "Expressions.h"
#include "../postgres_client/postgres_client.h"
#include <iostream>
#include <unordered_map>

table_t *HashJoin(table_t *left_table, table_t *right_table, u_int16_t l_col, u_int16_t r_col) {
    std::unordered_multimap<std::string, int> hashmap;
    for (int i = 0; i < left_table->num_tuples; i++) {
        // get the i'th tuple
        tuple_t *tupl = get_tuple(i, left_table);

        // hash row # -> store value at specified column
        // hashmap[tup1->field_list[l_col]] = i;
        switch (tupl->field_list[l_col].type) {
            case FIXEDCHAR:
                hashmap.insert(std::make_pair(tupl->field_list[l_col].f.fixed_char_field.val, i));
                break;
            case INT:
                // cast int to string to store in hashmap
                hashmap.insert(std::make_pair(std::to_string(tupl->field_list[l_col].f.int_field.val), i));
                break;
            case UNSUPPORTED:
                // TODO: figure out what to do here.
                break;
        }

    }

    // hash map should be built
    table_t *joined_table;
    //iterate over other table checking hash map
    for (int i = 0; i < right_table->num_tuples; i++) {
        tuple_t *tup2 = get_tuple(i, right_table);

        if (tup2->field_list[r_col].type == FIXEDCHAR)
        {
            auto range = hashmap.equal_range(tup2->field_list[r_col].f.fixed_char_field.val);
            for (auto it = range.first; it != range.second; ++it) {
                // TODO: do the join
            }
        }
        else if (tup2->field_list[r_col].type == INT)
        {
            auto range = hashmap.equal_range(std::to_string(tup2->field_list[r_col].f.int_field.val));
            for (auto it = range.first; it != range.second; ++it) { // for every value in the left that has that value, concat both.
                // TODO: do the join

            }
        }



        auto charthing = tup2->field_list[r_col].f.fixed_char_field.val;
        auto intthing = tup2->field_list[r_col].f.int_field.genval;

        // auto range = hashmap.equal_range(tup2->field_list[r_col].f.fixed_char_field.val);
        // match current tup column value to every row in first that is the same

    }


    return joined_table;
}


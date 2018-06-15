#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "vaultdb_generated.h"
#include "sgx_tcrypto.h"
using namespace std;

vector<vector<int>> simplejoin(vector<vector<int>> table1,
                               vector<vector<int>> table2, int join_column1_id,
                               int join_column2_id) {
  // table1 double for loop
  vector<int> join_column1 = table1[join_column1_id];
  vector<int> join_column2 = table2[join_column2_id];

  // got rid of the join column
  table1.erase(table1.begin() + join_column1_id);
  table2.erase(table2.begin() + join_column2_id);
  vector<vector<int>> other_table1 = table1;
  vector<vector<int>> other_table2 = table2;

  vector<int> common;

  vector<vector<int>> common_table1;
  vector<vector<int>> common_table2;
  vector<int> common_table1_ind;
  vector<int> common_table2_ind;

  vector<vector<int>> result;
  int join_size = join_column1.size();

  if (join_size != join_column2.size()) {
    throw "join columns are not the same size";
  }

  // get all the common indexes
  // use a set of indexes
  for (int i = 0; i < join_size; i++) {
    for (int j = 0; j < join_size; j++) {
      if (join_column1[i] == join_column2[j]) {
        common.push_back(join_column1[j]);
        common_table1_ind.push_back(i);
        common_table2_ind.push_back(j);
      }
    }
  }
  /*

  unordered_map<string, > hashmap;
  // iterate through the first table, getting the hash value of the join column
  as the key and the rest as the value. for (int i = 0; i < join_size; i++)
  {
      hashmap.insert(to_string(join_column1[i]));
  }
  */

  // for loop for getting inner table1
  for (int j = 0; j < other_table1.size(); j++) {

    vector<int> column;
    for (int i = 0; i < common_table1_ind.size(); i++) {
      column.push_back(other_table1[j][common_table1_ind[i]]);
    }
    common_table1.push_back(column);
  }

  // for loop for getting inner table2
  for (int j = 0; j < other_table2.size(); j++) {
    vector<int> column;
    for (int i = 0; i < common_table2_ind.size(); i++) {
      column.push_back(other_table2[j][i]);
    }
    common_table2.push_back(column);
  }

  // push_back all three to the result and return;
  result.insert(result.begin(), common_table2.begin(), common_table2.end());
  result.insert(result.begin(), common_table1.begin(), common_table1.end());
  result.push_back(common);
  return result;
}

uint8_t *hashjoin(uint8_t * left_table_buf, uint8_t * right_table_buf,
                  uint8_t * eq_left, uint8_t * eq_right)
{
    // make builder
    flatbuffers::FlatBufferBuilder builder(1024);
    // make tables
    auto left_table = flatbuffers::GetRoot<Table>(left_table_buf);
    auto right_table = flatbuffers::GetRoot<Table>(right_table_buf);
    // make tuple vector for new table
    vector<flatbuffers::Offset<Tuple> > tuple_vector;
    // make a new joined schema
    std::vector < flatbuffers::Offset<FieldDesc> > field_vector_desc;
    auto &leftfd_to_cp = *left_table->schema()->fielddescs();
    auto &rightfd_to_cp = *right_table->schema()->fielddescs();
    for (auto fd : leftfd_to_cp) {
      auto col_name = builder.CreateString(fd->name()->c_str());
      auto new_fd = CreateFieldDesc(builder, fd->type(), col_name, fd->col());
      field_vector_desc.push_back(new_fd);
    }
    for (auto fd : rightfd_to_cp) {
      auto col_name = builder.CreateString(fd->name()->c_str());
      auto new_fd = CreateFieldDesc(builder, fd->type(), col_name, fd->col());
      field_vector_desc.push_back(new_fd);
    }
    auto fielddescs = builder.CreateVectorOfSortedTables(&field_vector_desc);
    auto joined_schema = CreateSchema(builder, fielddescs);
    // make left right eq_expr
    auto left_expr = flatbuffers::GetRoot<Expr>(eq_left);
    auto right_expr = flatbuffers::GetRoot<Expr>(eq_right);
    // hash table for left
    map<uint32_t, vector<uint32_t> > left_hash;
    auto left_tuples = left_table->tuples();
    for (int i = 0; i < (int)left_tuples->Length(); i++) {
	auto each_tuple = left_tuples->Get(i);
	uint32_t hash_output = hash_field(each_tuple->fields()->Get(left_expr->expr.EqExpr->colno));
	pair<map<uint32_t,vector<uint32_t>>::iterator, bool> ret;
	ret = left_hash.insert(pair<uint32_t, vector<uint32_t>>(hash_output, vector<uint32_t>(1, i)));
	if(!ret.second) {
	    ret.first->second.push_back(i);
	}
    }
    // .find for the join column on the right. if found, 
    map<uint32_t, vector<uint32_t> >::iterator iter;
    for (auto right_tuple : *right_table->tuples()) {
	uint32_t hash_output = hash_field(right_tuple->fields()->Get(right_expr->expr().EqExpr->colno));
	iter = left_hash.find(hash_output);
	if(iter != left_hash.end()) {
	    // Now need to merge the tuples and add them to the new table.
	    for(int i : iter->second) {
		vector<flatbuffers::Offset<Field> > field_vector;
		auto left_tuple = left_table->tuples()->Get(i);
		for (auto left_field : *left_tuple->fields()) {
		    field_vector.push_back(left_field);
		}
		for (auto right_field: *right_tuple->fields()) {
		    field_vector.push_back(right_field);
		}
		auto row = builder.CreateVector(field_vector);
		auto new_tuple = CreateTuple(builder, joined_schema, row);
		bool is_dummy = left_tuple->isdummy() && right_tuple->isdummy();
		new_tuple.isdummy() = is_dummy;
		tuple_vector.push_back(field_vector);
	    }
	}
    }

    auto new_table = CreateTable(builder, joined_schema, tuple_vector);
    builder.Finish(new_table);
    uint8_t *buf = builder.GetBufferPointer();
    return buf;
}



#include <vector>
#include <cstdlib>
#include <map>
#include <iostream>
#include <ctime>
#include "vaultdb_generated.h"

int repart_step_one(uint8_t * table_buf) {
    uint64_t num_hosts = 10; //get_num_hosts_HB();
    std::srand(std::time(nullptr));
    auto table = flatbuffers::GetRoot<Table>(table_buf);
    std::vector<flatbuffers::FlatBufferBuilder> builders(num_hosts);
    auto schema = table->schema();
    auto tuples = table->tuples();
    std::map<int, std::vector<int>> rand_assignment;
    //TODO: make this process better
    // The reason we first populate a random map is 
    // so we can pack the Flatbuffers all at once. 
    for (int i = 0; i < tuples->Length(); i++) {
	rand_assignment[std::rand() % num_hosts].push_back(i);
    }

    

   
    for (std::map<int, std::vector<int>>::iterator 
	    it=rand_assignment.begin();
	    it!=rand_assignment.end(); it++) {

	uint8_t * buf = copy_tuples_by_index_list(tuples, it->second);
	add_buff_to_outbound_queue(buf);
    }
    send_out_repartition_step_one()
}

uint8_t * buf copy_tuples_by_index_list(flatbuffers::Vector<flatbuffers::Offset<Tuple>> tuples const, std::vector<int> indices) {
    
    flatbuffers::FlatBufferBuilder builder(1024);
    std::vector<flatbuffers::Offset<Tuple>> tuple_vector;
    for (auto i : indices) {
	auto tuple_to_cp = tuples.Get(i);
	std::vector<flatbuffers::Offset<Field>> field_vector;
	for (auto field_to_cp : tuple_to_cp->fields()) {
	    flatbuffers::Offset<Field> field_offset = 
		cp_field_from_const_field(field_to_cp, builder);
	    field_vector.push_back(field_offset);
	}
	auto row = builder.CreateVector(field_vector);
	auto tuple = CreateTuple(builder,schema_offset, row);
	tuple_vector.push_back(tuple);
    }
}
/*
int repart_step_two(std::vector<uint8_t *> tables) {

    uint64_t num_equiv_classes = 10; //get_num_equiv_HB();
    for (auto t : tables) {

    }
}
*/


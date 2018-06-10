#include "postgres_client.h"
#include <pqxx/pqxx>
#include "flatbuffers/minireflect.h"

pqxx::result query(std::string query_string, std::string dbname){ 
    pqxx::connection c(dbname);
    pqxx::work txn(c);
    pqxx::result res = txn.exec(query_string);
    return res;
}

FieldType get_OID_field_type(pqxx::oid oid) {
    switch (oid) {
	case VARCHAROID:
	    return FieldType_VARCHAR;
	case INT4OID:
	    return FieldType_INT;
	case TIMESTAMPOID:
	    return FieldType_TIMESTAMP;
	default:
	    return FieldType_UNSUPPORTED;
    }
}

flatbuffers::Offset<Schema> get_schema_offset_from_query(pqxx::result res, flatbuffers::FlatBufferBuilder &builder) {
    pqxx::row::size_type num_columns = res.columns();
    std::vector<flatbuffers::Offset<FieldDesc>> field_vector_desc;
    for (int i = 0; i < num_columns; i++) {
	auto column_name = builder.CreateString(res.column_name(i));
	auto field_desc = CreateFieldDesc(builder, get_OID_field_type(res.column_type(i)), 
				 column_name, i);
	field_vector_desc.push_back(field_desc);
    }
    auto fielddescs = builder.CreateVectorOfSortedTables(&field_vector_desc);

    auto schema_offset = CreateSchema(builder, fielddescs);
    return schema_offset;
}

uint8_t * get_table_from_query(pqxx::result res) {

}

flatbuffers::Offset<Field> get_field_offset_from_query(pqxx::field f, flatbuffers::FlatBufferBuilder &b) {
     //the duplication is due to how flatbuffers must be constructed, child tables 
     //must be created before parent tables
    switch (f.type()) {
	case VARCHAROID: {
	    auto s = b.CreateString(f.as<std::string>());
	    auto var_char = CreateVarCharField(b, s);
	    FieldBuilder field_builder(b);
	    field_builder.add_fieldnum(f.num());
	    field_builder.add_val_type(FieldVal_VarCharField);
	    field_builder.add_val(var_char.Union());
	    auto field = field_builder.Finish();
	    return field;
	}
	case INT4OID: {
	    auto int_field = CreateIntField(b, f.as<int>());
	    FieldBuilder field_builder(b);
	    field_builder.add_fieldnum(f.num());
	    field_builder.add_val_type(FieldVal_IntField);
	    field_builder.add_val(int_field.Union());
	    auto field = field_builder.Finish();
	    return field;
	}
	case TIMESTAMPOID: {
	    auto time_stamp_field = CreateTimeStampField(b, f.as<int>());
	    FieldBuilder field_builder(b);
	    field_builder.add_fieldnum(f.num());
	    field_builder.add_val_type(FieldVal_TimeStampField);
	    field_builder.add_val(time_stamp_field.Union());
	    auto field = field_builder.Finish();
	    return field;
        }
    }
}

uint8_t * postgres_query_writer(std::string query_string, std::string dbname) {
    pqxx::result res = query(query_string, dbname);
    flatbuffers::FlatBufferBuilder builder(1024);
    builder.ForceDefaults(true);
    auto schema_offset = get_schema_offset_from_query(res, builder);
    //auto schema = flatbuffers::GetRoot<Schema>(schema_buf);

    std::vector<flatbuffers::Offset<Tuple>> tuple_vector;
    for (auto psql_row: res) {
	std::vector<flatbuffers::Offset<Field>> field_vector;
	for (auto psql_field: psql_row) {
	    flatbuffers::Offset<Field> field_offset = 
		get_field_offset_from_query(psql_field, builder);
	    field_vector.push_back(field_offset);
	}
	auto row = builder.CreateVector(field_vector);
	auto tuple = CreateTuple(builder, schema_offset, row);
	tuple_vector.push_back(tuple);
    }
    auto tuples = builder.CreateVector(tuple_vector);
    auto table = CreateTable(builder, schema_offset, tuples);
    builder.Finish(table);
    uint8_t *buf = builder.GetBufferPointer();
    return buf;
    /*

    auto s = flatbuffers::FlatBufferToString(buf, TableTypeTable());
    std::cout << s << std::endl;
    */
}

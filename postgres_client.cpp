#include "postgres_client.h"
#include <glog/logging.h>
#include "flatbuffers/minireflect.h"


pqxx::result query(std::string query_string, std::string dbname) {
    pqxx::connection c(dbname);
    pqxx::work txn(c);
    pqxx::result res = txn.exec(query_string);
    return res;
}

FIELD_TYPE get_OID_field_type(pqxx::oid oid) {
    switch (oid) {
        case VARCHAROID:
            return FIXEDCHAR;
        case INT4OID:
            return INT;
        default:
            return UNSUPPORTED;
    }
}

table_builder_t *table_builder(std::string query_string, std::string dbname) {
    table_builder_t *tb = (table_builder_t *) malloc(sizeof(table_builder_t));
    tb->res = query(query_string, dbname);
    tb->expected_tuples = tb->res.capacity();
    tb->num_columns = tb->res.columns();
    LOG_IF(INFO, tb->num_columns > MAX_FIELDS) << "Max fields exceeded num columns:" << tb->num_columns;
    get_schema_from_query(tb);
}

schema_t *get_schema_from_query(table_builder_t *tb) {
    schema_t *schema = &t->schema;
    for (int i = 0; i < tb->num_columns; i++) {
        strncpy(schema->fields[i].field_name, tb->res.column_name(i), FIELD_NAME_LEN);
        schema->fields[i].col_no = (uint32_t) i;
        schema->fields[i].type = get_OID_field_type(tb->res.column_type(i));
        schema->num_fields++;
    }
    DLOG(INFO) << "Completed Schema with columns: " << num_columns;
    return schema;
}


uint8_t *get_table_from_query(pqxx::result res) {}

flatbuffers::Offset<Field>
get_field_offset_from_query(pqxx::field f, flatbuffers::FlatBufferBuilder &b) {
    // the duplication is due to how flatbuffers must be constructed, child tables
    // must be created before parent tables
    switch (f.type()) {
        case VARCHAROID: {
            auto s = b.CreateString(f.as<std::string>());
            auto var_char = CreateVarCharField(b, s);
            FieldBuilder field_builder(b);
            field_builder.add_fieldnum(f.num());
            field_builder.add_val_type(FieldVal_VarCharField);
            field_builder.add_val(var_char.Union());
            field_builder.add_genval_type(FieldVal_VarCharField);
            field_builder.add_genval(var_char.Union());
            auto field = field_builder.Finish();
            return field;
        }
        case INT4OID: {
            auto int_field = CreateIntField(b, f.as<int>());
            FieldBuilder field_builder(b);
            field_builder.add_fieldnum(f.num());
            field_builder.add_val_type(FieldVal_IntField);
            field_builder.add_val(int_field.Union());
            field_builder.add_genval_type(FieldVal_IntField);
            field_builder.add_genval(int_field.Union());
            auto field = field_builder.Finish();
            return field;
        }
        case TIMESTAMPOID: {
            auto time_stamp_field = CreateTimeStampField(b, f.as<int>());
            FieldBuilder field_builder(b);
            field_builder.add_fieldnum(f.num());
            field_builder.add_val_type(FieldVal_TimeStampField);
            field_builder.add_val(time_stamp_field.Union());
            field_builder.add_genval_type(FieldVal_TimeStampField);
            field_builder.add_genval(time_stamp_field.Union());
            auto field = field_builder.Finish();
            return field;
        }
    }
}

tuple_t *make_tuple(table_t *table, pqxx::tuple tuple) {
    for (auto psql_field : tuple) {
        switch (psql_field.type()) {
            case VARCHAROID: {
                table->

            }
            case INT4OID: {

            }
            case TIMESTAMPOID: {

            }
            default: {
                LOG(FATAL) << "Unsupported Type encountered" << psql_field.type();
            }

        }
    }

}

flatbuffers::DetachedBuffer postgres_query_writer(std::string query_string,
                                                  std::string dbname) {
    /*flatbuffers::FlatBufferBuilder builder(1024);*/

    /*builder.ForceDefaults(true);*/
    //auto schema_offset = get_schema_from_query(res);
    // auto schema = flatbuffers::GetRoot<Schema>(schema_buf);

    /*
std::vector<flatbuffers::Offset<Tuple>> tuple_vector;
for (auto psql_row : res) {
std::vector<flatbuffers::Offset<Field>> field_vector;
for (auto psql_field : psql_row) {
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
auto detached_buf = builder.Release();
     */
    //return detached_buf;
}

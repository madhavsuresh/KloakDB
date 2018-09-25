#include "postgres_client.h"
#include <glog/logging.h>
#include <iostream>
#include "flatbuffers/minireflect.h"

uint64_t tuples_per_page(uint64_t page_size, uint64_t tuple_size) {
    return (PAGE_SIZE - sizeof(uint64_t)) /tuple_size;
}

 void print_tuple(tuple_t * t) {
     for (int i = 0; i < t->num_fields; i++) {
         switch (t->field_list[i].type) {
             case FIXEDCHAR: {
                 printf("%s", t->field_list[i].f.fixed_char_field.val);
                 break;
             }
             case INT : {
                 printf("%d", t->field_list[i].f.int_field.val);
                 break;
             }
             case UNSUPPORTED : {
                 printf("ERROR, Unsupported type in print");
                 break;
             }
         }
         printf("| ");
     }
 }

expr_t make_int_expr(FILTER_EXPR type, uint64_t field_val, int colno) {
    expr_t expr;
    expr.field_val.type = INT;
    expr.field_val.f.int_field.val = field_val;
    expr.colno = colno;
    expr.expr_type = type;
    return expr;
}

pqxx::result query(std::string query_string, std::string dbname) {
    try {
        pqxx::connection c(dbname);
    } catch (const std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    pqxx::connection c(dbname);
    pqxx::work txn(c);
    pqxx::result res;
    try {
        res = txn.exec(query_string);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    txn.commit();
    txn.commit();
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

void free_table(table_t * t) {
    //TODO(madhavsuresh): there is a memory leak here!
    for (int i = 0; i < t->num_tuple_pages; i++) {
        free(t->tuple_pages[i]);
    }
    free(t);

}

tuple_page_t * allocate_tuple_page(table_builder_t * tb) {
    tb->table->tuple_pages[tb->curr_page] = (tuple_page_t *) malloc(PAGE_SIZE);
    bzero(tb->table->tuple_pages[tb->curr_page], PAGE_SIZE);
    tb->table->tuple_pages[tb->curr_page]->page_no = tb->curr_page;
    tb->num_pages_allocated++;
    tb->table->num_tuple_pages++;
    return tb->table->tuple_pages[tb->curr_page];
}

tuple_page_t * initialize_tuple_page(table_builder_t * tb) {
    tb->curr_page = 0;
    allocate_tuple_page(tb);
}

tuple_page_t * add_tuple_page(table_builder_t * tb) {
    tb->curr_page++;
    allocate_tuple_page(tb);

}

tuple_t * get_tuple_from_page(int page_tuple_num, tuple_page_t * tp, table_t * table) {
    return (tuple_t *)(((char *)tp->tuple_list)+(table->size_of_tuple*page_tuple_num));
}

tuple_t * get_tuple(int tuple_number, table_t * table) {
    int num_tuples_per_page = (int)tuples_per_page(PAGE_SIZE,table->size_of_tuple);
    int page_num = (tuple_number)/(num_tuples_per_page);
    tuple_page_t * tp = table->tuple_pages[page_num];
    int page_tuple_num = tuple_number % num_tuples_per_page;
    return (tuple_t *)(((char *)tp->tuple_list)+(table->size_of_tuple*page_tuple_num));
}

void write_table_from_postgres(pqxx::result res, table_builder_t * tb) {
    tuple_page_t *curr_tp = tb->table->tuple_pages[tb->curr_page];
    for (auto psql_row : res) {
        // Don't want to jump on the first tuple
        if (0 == tb->curr_tuple % tb->num_tuples_per_page && tb->curr_tuple > 0) {
            curr_tp = add_tuple_page(tb);
        }
        tuple_t *tuple = get_tuple(tb->curr_tuple, tb->table);
        //tuple_t *tuple = get_tuple_from_page(curr_tp->num_tuples, curr_tp, tb->table);
        tb->table->num_tuples++;
        int field_counter = 0;
        tuple->num_fields = tb->table->schema.num_fields;
        for (auto field : psql_row) {
            switch (tb->table->schema.fields[field_counter].type) {
                case FIXEDCHAR:
                    strncpy(tuple->field_list[field_counter].f.fixed_char_field.val, field.c_str(), FIXEDCHAR_LEN);
                    tuple->field_list[field_counter].type = FIXEDCHAR;
                    break;
                case INT:
                    tuple->field_list[field_counter].f.int_field.val = field.as<int>();
                    tuple->field_list[field_counter].type = INT;
                    break;
                case UNSUPPORTED:
                    printf("ERROR!!");
                    break;
            }
            field_counter++;
        }
        tb->curr_tuple++;
        fflush(stdin);
    }
}

table_t * allocate_table(int num_tuple_pages) {
    return (table_t*)malloc(sizeof(table_t) + num_tuple_pages* sizeof(tuple_page_t *));
}

void init_table_builder(pqxx::result res ,table_builder_t * tb) {

    tb->expected_tuples = res.capacity();
    tb->num_columns = res.columns();
    //DLOG_IF(INFO, tb->num_columns > MAX_FIELDS) << "Max fields exceeded num columns:" << tb->num_columns;
    schema_t *schema = get_schema_from_query(tb, res);
    tb->size_of_tuple = sizeof(tuple) +
                        schema->num_fields * (sizeof(field_t));

    uint64_t total_size = tb->expected_tuples * tb->size_of_tuple;
    tb->expected_pages = tb->expected_tuples/tuples_per_page(PAGE_SIZE, tb->size_of_tuple) + 1;
   // total_size / PAGE_SIZE + 2;
   //TODO(madhavsuresh): this needs to be abstracted out. this is terrible.
    tb->table = allocate_table(tb->expected_pages); //(table_t *) malloc(sizeof(table_t) + sizeof(tuple_page_t *) * tb->expected_pages);
    bzero(tb->table, sizeof(table_t) + sizeof(tuple_page_t *) * tb->expected_pages);
    // Copy schema to new table
    memcpy(&tb->table->schema, schema, sizeof(schema_t));
    free(schema);

    tb->table->size_of_tuple = tb->size_of_tuple;
    tb->num_tuples_per_page = tuples_per_page(PAGE_SIZE, tb->table->size_of_tuple);
    tb->curr_tuple = 0;
    tb->curr_page = 0;

    // Initialize first page regardless
    initialize_tuple_page(tb);
}


table_builder_t *table_builder(std::string query_string, std::string dbname) {
    auto *tb = (table_builder_t *) malloc(sizeof(table_builder_t));
    bzero(tb, sizeof(table_builder_t));

    pqxx::result res = query(query_string, dbname);

    init_table_builder(res, tb);
    write_table_from_postgres(res,tb);


    // Everything should be zero-indexed
    return tb;
}


schema_t *get_schema_from_query(table_builder_t *tb, pqxx::result res) {
    schema_t *schema =  (schema_t *) malloc(sizeof(schema_t));
    schema->num_fields = res.columns();
    for (int i = 0; i < tb->num_columns; i++) {
        strncpy(schema->fields[i].field_name, res.column_name(i), FIELD_NAME_LEN);
        schema->fields[i].col_no = (uint32_t) i;
        schema->fields[i].type = get_OID_field_type(res.column_type(i));
    }
    //DLOG(INFO) << "Completed Schema with columns: " << tb->num_columns;
    return schema;
}

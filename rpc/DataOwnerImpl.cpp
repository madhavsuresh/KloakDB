//
// Created by madhav on 10/1/18.
//

#include "DataOwnerImpl.h"
#include "DataOwnerPrivate.h"
#include "Repartition.h"
#include "data/pqxx_compat.h"
#include "logger/Logger.h"
#ifndef LOGGER_DEFS
#define LOGGER_DEFS
#include "logger/LoggerDefs.h"
#endif
#include "operators/Aggregate.h"
#include "operators/Filter.h"
#include "operators/Generalize.h"
#include "operators/HashJoin.h"
#include "operators/Sort.h"
#include "sgx/App/VaultDBSGXApp.h"
#include <future>

extern std::promise<void> exit_requested;

DataOwnerImpl::DataOwnerImpl(DataOwnerPrivate *p) { this->p = p; }

::grpc::Status
DataOwnerImpl::ShutDown(::grpc::ServerContext *context,
                        const ::vaultdb::ShutDownRequest *request,
                        ::vaultdb::ShutDownResponse *response) {
  for (int i = 0; i < p->NumHosts(); i++) {
    p->DeleteDataOwnerClient(i);
  }
  exit_requested.set_value();
  LOG(DO_IMPL) << "Shutdown OK (" << context->peer() << ")";
  return grpc::Status::OK;
}

::grpc::Status
DataOwnerImpl::GetPeerHosts(::grpc::ServerContext *context,
                            const ::vaultdb::GetPeerHostsRequest *request,
                            ::vaultdb::GetPeerHostsResponse *response) {
  for (int i = 0; i < request->hostnames_size(); i++) {
    auto host = request->hostnames(i);
    p->SetDataOwnerClient(host.hostnum(), host.hostname());
  }
  LOG(DO_IMPL) << "GetPeerHosts OK (" << context->peer() << ")";
  return grpc::Status::OK;
}

::grpc::Status DataOwnerImpl::RepartitionStepTwo(
    ::grpc::ServerContext *context,
    const ::vaultdb::RepartitionStepTwoRequest *request,
    ::vaultdb::RepartitionStepTwoResponse *response) {
  LOG(DO_IMPL) << "Repartition Step Two Start";
  START_TIMER(repart_step_two_full);
  std::vector<table_t *> table_ptrs;
  for (int i = 0; i < request->tablefragments_size(); i++) {
    table_ptrs.push_back(p->GetTable(request->tablefragments(i).tableid()));
  }
  START_TIMER(repart_step_two_inner);
  std::vector<std::pair<int32_t, int32_t>> info;
  if (table_ptrs.size() == 0) {
    LOG(DO_IMPL) << "Repartition Step Two, no input tables on host";
  } else {
    LOG(DO_IMPL) << "Repartition Step Two on #[" << table_ptrs.size()
                 << "] tables";
    info = repartition_step_two(table_ptrs, p->NumHosts(), p);
  }
  END_TIMER(repart_step_two_inner);

  for (auto i : info) {
    ::vaultdb::TableID *id = response->add_remoterepartitionids();
    id->set_hostnum(i.first);
    id->set_tableid(i.second);
  }
  END_TIMER(repart_step_two_full);
  LOG_TIMER(repart_step_two_full);
  LOG_TIMER(repart_step_two_inner);
  LOG(DO_IMPL) << "Repartition Step Two OK (" << context->peer() << ")";
  return grpc::Status::OK;
}

::grpc::Status DataOwnerImpl::RepartitionStepOne(
    ::grpc::ServerContext *context,
    const ::vaultdb::RepartitionStepOneRequest *request,
    ::vaultdb::RepartitionStepOneResponse *response) {
  LOG(DO_IMPL) << "Repartition Step One Start";
  START_TIMER(repart_step_one_full);
  table_t *t = p->GetTable(request->tableid().tableid());
  START_TIMER(repart_step_one_inner);
  std::vector<std::pair<int32_t, int32_t>> info =
      repart_step_one(t, p->NumHosts(), p);
  END_TIMER(repart_step_one_inner);
  p->FreeTable(request->tableid().tableid());
  for (auto i : info) {
    ::vaultdb::TableID *id = response->add_remoterepartitionids();
    id->set_hostnum(i.first);
    id->set_tableid(i.second);
  }
  END_TIMER(repart_step_one_full);
  LOG_TIMER(repart_step_one_inner);
  LOG_TIMER(repart_step_one_full);
  LOG(DO_IMPL) << "Repartition Step One OK (" << context->peer() << ")";
  return grpc::Status::OK;
}

void log_gen_zip(table_t *zipped, int colno) {
  map<int, int> counter;
  for (int i = 0; i < zipped->num_tuples; i++) {
    counter[get_tuple(i, zipped)->field_list[colno].f.int_field.genval]++;
  }
  int max_val = 0;
  int min_val = 100000;
  int num_classes = 0;
  for (auto &i : counter) {
    num_classes++;
    if (max_val < i.second) {
      max_val = i.second;
    }
    if (min_val > i.second) {
      min_val = i.second;
    }
  }
  LOG(STATS) << "Gen Stats: MIN:[" << min_val << "], MAX:[" << max_val
             << "] AVG:[" << (double)zipped->num_tuples / num_classes << "]";
}
::grpc::Status
DataOwnerImpl::GeneralizeZip(::grpc::ServerContext *context,
                             const ::vaultdb::GeneralizeZipRequest *request,
                             ::vaultdb::GeneralizeZipResponse *response) {
  START_TIMER(gen_zip_full);
  table_t *gen_table = p->GetTable(request->gentableid().tableid());
  table_t *scan_table = p->GetTable(request->scantableid().tableid());
  START_TIMER(gen_zip_inner);
  table_t *output_table = generalize_zip(
      scan_table, gen_table, colno_from_name(scan_table, request->colname()));
  log_gen_zip(output_table, colno_from_name(scan_table, request->colname()));
  END_TIMER(gen_zip_inner);

  auto tid = response->mutable_generalizedscantable();
  tid->set_hostnum(p->HostNum());
  tid->set_tableid(p->AddTable(output_table));
  END_TIMER(gen_zip_full);
  LOG_TIMER(gen_zip_inner);
  LOG_TIMER(gen_zip_full);
  LOG(DO_IMPL) << "GeneralizeZip OK (" << context->peer() << ")";
  return grpc::Status::OK;
}

::grpc::Status DataOwnerImpl::GetTable(
    ::grpc::ServerContext *context, const ::vaultdb::GetTableRequest *request,
    ::grpc::ServerWriter<::vaultdb::GetTableResponse> *writer) {

  START_TIMER(get_table_full);
  table_t *t = p->GetTable(request->id().tableid());

  ::vaultdb::GetTableResponse header;
  header.set_is_header(true);
  header.set_num_tuples(t->num_tuples);
  header.set_num_tuple_pages(t->num_tuple_pages);
  header.set_size_of_tuple(t->size_of_tuple);
  ::vaultdb::Schema *s = header.mutable_schema();
  table_schema_to_proto_schema(t, s);
  writer->Write(header);

  ::vaultdb::GetTableResponse pages;
  for (int i = 0; i < t->num_tuple_pages; i++) {
    pages.set_is_header(false);
    pages.set_page_no(i);
    pages.set_page((char *)t->tuple_pages[i], PAGE_SIZE);
    writer->Write(pages);
  }
  END_AND_LOG_RPC_TIMER(get_table_full, p->HostName());
  LOG(DO_IMPL) << "GetTable OK (" << context->peer() << ")";
  return grpc::Status::OK;
}

::grpc::Status DataOwnerImpl::SendTable(
    ::grpc::ServerContext *context,
    ::grpc::ServerReader<::vaultdb::SendTableRequest> *reader,
    ::vaultdb::SendTableResponse *response) {

  START_TIMER(send_table_full);
  vaultdb::SendTableRequest req;

  table_t *t;

  while (reader->Read(&req)) {

    if (req.is_header()) {
      t = allocate_table(req.num_tuple_pages());
      t->num_tuples = req.num_tuples();
      t->size_of_tuple = req.size_of_tuple();
      t->num_tuple_pages = req.num_tuple_pages();
      proto_schema_to_table_schema(t, req.schema());
    } else {
      t->tuple_pages[req.page_no()] = (tuple_page_t *)malloc(PAGE_SIZE);
      memcpy(t->tuple_pages[req.page_no()], req.page().c_str(), PAGE_SIZE);
    }
  }
  LOG(INFO) << "Adding table received from ??";
  int table_id = this->p->AddTable(t);
  response->set_tableid(table_id);
  END_AND_LOG_RPC_TIMER(send_table_full, p->HostName());
  LOG(DO_IMPL) << "Send Table OK (" << context->peer() << ")";
  return grpc::Status::OK;
}

::grpc::Status
DataOwnerImpl::DBMSQuery(::grpc::ServerContext *context,
                         const ::vaultdb::DBMSQueryRequest *request,
                         ::vaultdb::DBMSQueryResponse *response) {

  START_TIMER(dbms_query_full);
  START_TIMER(dbms_query_inner);
  table *t = get_table(request->query(), request->dbname());
  END_TIMER(dbms_query_inner);
  int table_id = this->p->AddTable(t);
  vaultdb::TableID *tid = response->mutable_tableid();
  tid->set_tableid(table_id);
  tid->set_hostnum(this->p->HostNum());
  tid->set_query(request->query());
  tid->set_dbname(request->dbname());
  END_TIMER(dbms_query_full);
  LOG_TIMER(dbms_query_inner);
  LOG_TIMER(dbms_query_full);
  LOG(DO_IMPL) << "Num Tuples" << t->num_tuples;
  LOG(DO_IMPL) << "Num Tuples (manager)" << p->GetTable(table_id)->num_tuples;
  LOG(DO_IMPL) << "DBMSQuery OK query: (" << request->query() << ")";

  return grpc::Status::OK;
}

::grpc::Status
DataOwnerImpl::FreeTable(::grpc::ServerContext *context,
                         const ::vaultdb::FreeTableRequest *request,
                         ::vaultdb::FreeTableResponse *response) {

  p->FreeTable(request->tid().tableid());
  LOG(DO_IMPL) << "Free Table OK (" << context->peer() << ")";
  return grpc::Status::OK;
}

::grpc::Status
DataOwnerImpl::CoalesceTables(::grpc::ServerContext *context,
                              const ::vaultdb::CoaleseTablesRequest *request,
                              ::vaultdb::CoaleseTablesResponse *response) {

  START_TIMER(coalesce_tables_full);
  std::vector<table_t *> tables;
  LOG(DO_IMPL) << "Coalescing #=[" << request->tablefragments_size()
               << "] tables";
  for (int i = 0; i < request->tablefragments_size(); i++) {
    tables.push_back(p->GetTable(request->tablefragments(i).tableid()));
  }
  START_TIMER(coalesce_tables_inner);
  table_t *t = coalesce_tables(tables);
  END_TIMER(coalesce_tables_inner);
  ::vaultdb::TableID *tid = response->mutable_id();
  tid->set_tableid(p->AddTable(t));
  tid->set_hostnum(p->HostNum());
  END_TIMER(coalesce_tables_full);
  LOG_TIMER(coalesce_tables_inner);
  LOG_TIMER(coalesce_tables_full);
  LOG(OP) << "Coalesce Tables Size: [" << t->num_tuples << "]";
  LOG(DO_IMPL) << "Coalesce Table OK (" << context->peer() << ")";
  return grpc::Status::OK;
}

expr_t make_expr_t(table_t *t, const ::vaultdb::Expr &expr) {
  LOG(DO_IMPL) << "Making filter expression";
  expr_t ex;
  ex.colno = colno_from_name(t, expr.colname());
  switch (expr.type()) {
  case vaultdb::Expr_ExprType_EQ_EXPR: {
    ex.expr_type = EQ_EXPR;
    LOG(DO_IMPL) << "EQ filter expression";
    break;
  }
  case Expr_ExprType_NEQ_EXPR: {
    ex.expr_type = NEQ_EXPR;
    break;
  }
  case Expr_ExprType_LIKE_EXPR: {
    LOG(DO_IMPL) << "LIKE filter expression";
    ex.expr_type = LIKE_EXPR;
    break;
  }
  default: {
    LOG(DO_IMPL) << "THROW UNSUPPORTED filter expression";
    throw; }
  }

  switch (expr.desc().field_type()) {
  case ::vaultdb::FieldDesc_FieldType_FIXEDCHAR: {
    ex.field_val.type = FIXEDCHAR;
    strncpy(ex.field_val.f.fixed_char_field.val, expr.charfield().c_str(),
            FIXEDCHAR_LEN);
    break;
  }
  case ::vaultdb::FieldDesc_FieldType_INT: {
    ex.field_val.type = INT;
    ex.field_val.f.int_field.val = (int64_t)expr.intfield();
    break;
  }
  default:
    LOG(DO_IMPL) << "THROW UNSUPPORTED filter desc type";
    throw;
  }
  return ex;
}

::grpc::Status DataOwnerImpl::KFilter(::grpc::ServerContext *context,
                                      const ::vaultdb::KFilterRequest *request,
                                      ::vaultdb::KFilterResponse *response) {
  START_TIMER(filter_full);
  table_t *in = p->GetTable(request->tid().tableid());
  expr_t ex = make_expr_t(in, request->expr());
  table_t *f;
  if (request->in_sgx()) {
    LOG(DO_IMPL) << "SGX Filter";
    START_TIMER(sgx_filter_inner);
    f = filter_sgx(in, &ex);
    END_TIMER(sgx_filter_inner);
    LOG_TIMER(sgx_filter_inner);
  } else {
    LOG(DO_IMPL) << "Plain Filter";
    START_TIMER(plain_filter_inner);
    f = filter(in, &ex);
    END_TIMER(plain_filter_inner);
    LOG_TIMER(plain_filter_inner);
  }
  auto tid = response->mutable_tid();
  tid->set_hostnum(p->HostNum());
  tid->set_tableid(p->AddTable(f));
  END_TIMER(filter_full);
  LOG_TIMER(filter_full);
  LOG(DO_IMPL) << "Filter OK";
  return ::grpc::Status::OK;
}

sort_t make_sort_t(table_t *t, const ::vaultdb::SortDef sort) {
  sort_t s;
  s.colno = colno_from_name(t, sort.colname());
  s.ascending = sort.ascending();
  return s;
}

::grpc::Status DataOwnerImpl::KSort(::grpc::ServerContext *context,
                                    const ::vaultdb::KSortRequest *request,
                                    ::vaultdb::KSortResponse *response) {
  START_TIMER(sort_full);
  table_t *in = p->GetTable(request->tid().tableid());
  sort_t s = make_sort_t(in, request->sortdef());

  table_t *sorted;
  if (request->in_sgx()) {
    LOG(DO_IMPL) << "SGX Sort";
    START_TIMER(sgx_sort_inner);
    sorted = sort_sgx(in, &s);
    END_TIMER(sgx_sort_inner);
    LOG_TIMER(sgx_sort_inner);
  } else {
    LOG(DO_IMPL) << "Plain Sort";
    START_TIMER(plain_sort_inner);
    sorted = sort(in, &s);
    END_TIMER(plain_sort_inner);
    LOG_TIMER(plain_sort_inner);
  }
  auto tid = response->mutable_tid();
  tid->set_hostnum(p->HostNum());
  tid->set_tableid(p->AddTable(sorted));
  END_TIMER(sort_full);
  LOG_TIMER(sort_full);
  LOG(DO_IMPL) << "Sort OK";
  return ::grpc::Status::OK;
}

join_def_t make_join_def_t(table_t *left, table_t *right,
                           ::vaultdb::JoinDef def) {
  join_def_t def_t;
  def_t.l_col = colno_from_name(left, def.l_col_name());
  def_t.r_col = colno_from_name(right, def.r_col_name());
  def_t.project_len = def.project_len();

  for (int i = 0; i < def.project_list_size(); i++) {
    auto &project = def.project_list(i);
    switch (project.side()) {
    case ::vaultdb::JoinColID_RelationSide_LEFT: {
      def_t.project_list[i].side = LEFT_RELATION;
      def_t.project_list[i].col_no = colno_from_name(left, project.colname());
      break;
    }
    case ::vaultdb::JoinColID_RelationSide_RIGHT: {
      def_t.project_list[i].side = RIGHT_RELATION;
      def_t.project_list[i].col_no = colno_from_name(right, project.colname());
      break;
    }
    default: { throw; }
    }
  }
  return def_t;
}

::grpc::Status DataOwnerImpl::MakeObli(::grpc::ServerContext* context, const ::vaultdb::MakeObliRequest* request,
        ::vaultdb::MakeObliResponse* response) {
  START_TIMER(make_obli_full);
  table_t *t = p->GetTable(request->tid().tableid());


  for (int j = 0; j < t->num_tuples; j++) {
    switch (t->schema.fields[colno_from_name(t, request->col_name())].type) {
      case FIXEDCHAR: {
        get_tuple(j, t)->field_list[colno_from_name(t, request->col_name())].f.fixed_char_field.genval = 0;
        break;
      }
      case INT : {
        get_tuple(j, t)->field_list[colno_from_name(t, request->col_name())].f.int_field.genval = 0;
        break;
      }
      default: {
        throw;
      }
    }
  }
  auto tid = response->mutable_tid();
  tid->set_hostnum(p->HostNum());
  tid->set_tableid(p->AddTable(t));
  END_AND_LOG_TIMER(make_obli_full);
  return ::grpc::Status::OK;
}

::grpc::Status DataOwnerImpl::KJoin(::grpc::ServerContext *context,
                                    const ::vaultdb::KJoinRequest *request,
                                    ::vaultdb::KJoinResponse *response) {
  START_TIMER(join_full);
  table_t *left = p->GetTable(request->left_tid().tableid());
  table_t *right = p->GetTable(request->right_tid().tableid());
  join_def_t def = make_join_def_t(left, right, request->def());
  table_t *out_join;
  if (request->in_sgx()) {
    LOG(DO_IMPL) << "SGX HashJoin";
    START_TIMER(sgx_join_inner);
    out_join = hash_join_sgx(left, right, def);
    END_TIMER(sgx_join_inner);
    LOG_TIMER(sgx_join_inner);
  } else {
    START_TIMER(plain_join_inner);
    LOG(DO_IMPL) << "Plain HashJoin";
    out_join = hash_join(left, right, def);
    END_TIMER(plain_join_inner);
    LOG_TIMER(plain_join_inner);
  }
  LOG(OP) << "Join Number of Output Tuples :[" << out_join->num_tuples << "]";
  LOG(OP) << "Join Number of Tuples Pages:[" << out_join->num_tuple_pages << "]";
  auto tid = response->mutable_tid();
  tid->set_hostnum(p->HostNum());
  tid->set_tableid(p->AddTable(out_join));
  END_TIMER(join_full);
  LOG_TIMER(join_full);
   p->FreeTable(request->left_tid().tableid());
  p->FreeTable(request->right_tid().tableid());
  LOG(DO_IMPL) << "Join OK";
  return ::grpc::Status::OK;
}

groupby_def_t make_groupby_def_t(table_t *t, ::vaultdb::GroupByDef def) {
  groupby_def_t def_t;

  // TODO(madhavsuresh): this needs to be fixed, the API is inconsistent.
  switch (def.type()) {
  case ::vaultdb::GroupByDef_GroupByType_COUNT: {
    def_t.type = COUNT;
    def_t.colno = colno_from_name(t, def.col_name());
    break;
  }
  case ::vaultdb::GroupByDef_GroupByType_MINX: {
    def_t.type = MINX;
    def_t.colno = colno_from_name(t, def.col_name());
    break;
  }
  case ::vaultdb::GroupByDef_GroupByType_AVG: {
    def_t.type = AVG;
    def_t.colno = colno_from_name(t, def.col_name());
    LOG(OP) << "AVG COL NO" << def_t.colno << " NAME:" << def.col_name();
    LOG(OP) << "AVG NUM GBCOLS" <<  def.gb_col_names_size();
    for (int i = 0; i < def.gb_col_names_size(); i++) {
      def_t.gb_colnos[i] =
          static_cast<uint8_t>(colno_from_name(t, def.gb_col_names(i)));
      LOG(OP) << " GB NAMES:" << def.gb_col_names(i) << ", NO" << def_t.gb_colnos[i];
    }
    def_t.num_cols = def.gb_col_names_size();
    break;
  }
  default: { throw; }
  }
  return def_t;
}

::grpc::Status
DataOwnerImpl::KAggregate(::grpc::ServerContext *context,
                          const ::vaultdb::KAggregateRequest *request,
                          ::vaultdb::KAggregateResponse *response) {
  START_TIMER(aggregate_full);
  table_t *in = p->GetTable(request->tid().tableid());
  groupby_def_t gbd = make_groupby_def_t(in, request->def());
  table_t *out;
  if (request->in_sgx()) {
    LOG(DO_IMPL) << "SGX Aggregate";
    START_TIMER(sgx_aggregate_inner);
    out = aggregate_sgx(in, &gbd);
    END_TIMER(sgx_aggregate_inner);
    LOG_TIMER(sgx_aggregate_inner);
  } else {
    LOG(DO_IMPL) << "Plain Aggregate";
    START_TIMER(plain_aggregate_inner);
    out = aggregate(in, &gbd);
    END_TIMER(plain_aggregate_inner);
    LOG_TIMER(plain_aggregate_inner);
  }
  LOG(OP) << "Aggregate Number of Output Tuples :[" << out->num_tuples << "]";
  auto tid = response->mutable_tid();
  tid->set_hostnum(p->HostNum());
  tid->set_tableid(p->AddTable(out));
  END_TIMER(aggregate_full);
  LOG_TIMER(aggregate_full);
  LOG(DO_IMPL) << "Aggregate OK";
  return ::grpc::Status::OK;
}
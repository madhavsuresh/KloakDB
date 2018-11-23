//
// Created by madhav on 10/1/18.
//

#include <operators/Generalize.h>
#include "DataOwnerImpl.h"
#include "operators/Aggregate.h"
#include "operators/Filter.h"
#include "operators/HashJoin.h"
#include "logger/Logger.h"
#include "Repartition.h"
#include "operators/Sort.h"
#include "data/pqxx_compat.h"
#include "DataOwnerPrivate.h"
#include "sgx/App/VaultDBSGXApp.h"

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
  return grpc::Status::OK;
}

::grpc::Status DataOwnerImpl::RepartitionStepTwo(
    ::grpc::ServerContext *context,
    const ::vaultdb::RepartitionStepTwoRequest *request,
    ::vaultdb::RepartitionStepTwoResponse *response) {
  std::vector<table_t *> table_ptrs;
  for (int i = 0; i < request->tablefragments_size(); i++) {
    table_ptrs.push_back(p->GetTable(request->tablefragments(i).tableid()));
  }
  std::vector<std::pair<int32_t, int32_t>> info =
      repartition_step_two(table_ptrs, p->NumHosts(), p);
  for (auto i : info) {
    ::vaultdb::TableID *id = response->add_remoterepartitionids();
    id->set_hostnum(i.first);
    id->set_tableid(i.second);
  }
  return grpc::Status::OK;
}

::grpc::Status DataOwnerImpl::RepartitionStepOne(
    ::grpc::ServerContext *context,
    const ::vaultdb::RepartitionStepOneRequest *request,
    ::vaultdb::RepartitionStepOneResponse *response) {
  table_t *t = p->GetTable(request->tableid().tableid());
  std::vector<std::pair<int32_t, int32_t>> info =
      repart_step_one(t, p->NumHosts(), p);
  for (auto i : info) {
    ::vaultdb::TableID *id = response->add_remoterepartitionids();
    id->set_hostnum(i.first);
    id->set_tableid(i.second);
  }
  return grpc::Status::OK;
}
::grpc::Status DataOwnerImpl::GeneralizeZip(::grpc::ServerContext* context, const ::vaultdb::GeneralizeZipRequest* request,
        ::vaultdb::GeneralizeZipResponse* response) {
  table_t *gen_table = p->GetTable(request->gentableid().tableid());
  table_t *scan_table = p->GetTable(request->scantableid().tableid());
  table_t *output_table = generalize_zip(scan_table, gen_table, request->colno());

  auto tid = response->mutable_generalizedscantable();
  tid->set_hostnum(p->HostNum());
  tid->set_tableid(p->AddTable(output_table));
  return grpc::Status::OK;
}

::grpc::Status DataOwnerImpl::GetTable(::grpc::ServerContext* context, const ::vaultdb::GetTableRequest* request,
        ::grpc::ServerWriter< ::vaultdb::GetTableResponse>* writer) {

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
  return grpc::Status::OK;
}


::grpc::Status DataOwnerImpl::SendTable(
    ::grpc::ServerContext *context,
    ::grpc::ServerReader<::vaultdb::SendTableRequest> *reader,
    ::vaultdb::SendTableResponse *response) {

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
  return grpc::Status::OK;
}

::grpc::Status
DataOwnerImpl::DBMSQuery(::grpc::ServerContext *context,
                         const ::vaultdb::DBMSQueryRequest *request,
                         ::vaultdb::DBMSQueryResponse *response) {

  table *t = get_table(request->query(), request->dbname());
  LOG(INFO) << "Adding Table from Query:" << request->query();
  int table_id = this->p->AddTable(t);
  vaultdb::TableID *tid = response->mutable_tableid();
  tid->set_tableid(table_id);
  tid->set_hostnum(this->p->HostNum());
  tid->set_query(request->query());
  tid->set_dbname(request->dbname());

  return grpc::Status::OK;
}

::grpc::Status
DataOwnerImpl::CoalesceTables(::grpc::ServerContext *context,
                              const ::vaultdb::CoaleseTablesRequest *request,
                              ::vaultdb::CoaleseTablesResponse *response) {

  std::vector<table_t *> tables;
  for (int i = 0; i < request->tablefragments_size(); i++) {
    LOG(INFO) << "Coalescing" << request->tablefragments(i).tableid();
    tables.push_back(p->GetTable(request->tablefragments(i).tableid()));
  }
  table_t *t = coalesce_tables(tables);
  ::vaultdb::TableID *tid = response->mutable_id();
  tid->set_tableid(p->AddTable(t));
  tid->set_hostnum(p->HostNum());
  LOG(INFO) << "After coalescing: " << p->GetTable(tid->tableid())->num_tuples;
  return grpc::Status::OK;
}

expr_t make_expr_t(table_t * t, const ::vaultdb::Expr &expr) {
  expr_t ex;
  ex.colno = colno_from_name(t, expr.colname());
  switch (expr.type()) {
  case vaultdb::Expr_ExprType_EQ_EXPR: {
    ex.expr_type = EQ_EXPR;
    break;
  }
  default: { throw; }
  }
  switch (expr.desc().field_type()) {
  case ::vaultdb::FieldDesc_FieldType_FIXEDCHAR: {
    ex.field_val.type = FIXEDCHAR;
    memcpy(ex.field_val.f.fixed_char_field.val, expr.charfield().c_str(),
           FIXEDCHAR_LEN);
    break;
  }
  case ::vaultdb::FieldDesc_FieldType_INT: {
    ex.field_val.type = INT;
    ex.field_val.f.int_field.val = (int64_t)expr.intfield();
    break;
  }
  default:
    throw;
  }
  return ex;
}

::grpc::Status DataOwnerImpl::KFilter(::grpc::ServerContext *context,
                                      const ::vaultdb::KFilterRequest *request,
                                      ::vaultdb::KFilterResponse *response) {
  table_t *in = p->GetTable(request->tid().tableid());
  expr_t ex = make_expr_t(in, request->expr());
  table_t *f;
  if (request->in_sgx()) {
    f = filter_sgx(in, &ex);
  } else {
    f = filter(in, &ex);
  }
  auto tid = response->mutable_tid();
  tid->set_hostnum(p->HostNum());
  tid->set_tableid(p->AddTable(f));
  LOG(INFO) << "Success";
  for (int i = 0; i < f->num_tuples; i++) {
    print_tuple_log(i, get_tuple(i, f));
  }
  return ::grpc::Status::OK;
}

sort_t make_sort_t(table_t * t, const ::vaultdb::SortDef sort) {
  sort_t s;
  s.colno = colno_from_name(t, sort.colname());
  s.ascending = sort.ascending();
  return s;
}

::grpc::Status DataOwnerImpl::KSort(::grpc::ServerContext *context,
                                    const ::vaultdb::KSortRequest *request,
                                    ::vaultdb::KSortResponse *response) {
  table_t * in = p->GetTable(request->tid().tableid());
  sort_t s = make_sort_t(in, request->sortdef());

  table_t *sorted;
  if (request->in_sgx()) {
    sorted = sort_sgx(in, &s);
  } else {
    sorted = sort(in, &s);
  }
  auto tid = response->mutable_tid();
  tid->set_hostnum(p->HostNum());
  tid->set_tableid(p->AddTable(sorted));
  LOG(INFO) << "Success";
  for (int i = 0; i < sorted->num_tuples; i++) {
    print_tuple_log(i, get_tuple(i, sorted));
  }
  return ::grpc::Status::OK;
}

join_def_t make_join_def_t(table_t * left, table_t * right, ::vaultdb::JoinDef def) {
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

::grpc::Status DataOwnerImpl::KJoin(::grpc::ServerContext *context,
                                    const ::vaultdb::KJoinRequest *request,
                                    ::vaultdb::KJoinResponse *response) {
  table_t *left = p->GetTable(request->left_tid().tableid());
  table_t *right = p->GetTable(request->right_tid().tableid());
  join_def_t def = make_join_def_t(left, right, request->def());
  table_t *out_join;
  if (request->in_sgx()) {
    out_join = hash_join_sgx(left, right, def);
  } else {

    out_join = hash_join(left, right, def);
  }
  auto tid = response->mutable_tid();
  tid->set_hostnum(p->HostNum());
  tid->set_tableid(p->AddTable(out_join));
  LOG(INFO) << "Success";
  for (int i = 0; i < out_join->num_tuples; i++) {
    print_tuple_log(i, get_tuple(i, out_join));
  }
  LOG(INFO) << out_join->num_tuples;
  return ::grpc::Status::OK;
}

groupby_def_t make_groupby_def_t(table_t * t, ::vaultdb::GroupByDef def) {
  groupby_def_t def_t;

  //TODO(madhavsuresh): this needs to be fixed, the API is inconsistent.
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
    def_t.colno = def.col_no();
    for (int i = 0; i < def.gb_col_nos_size(); i++) {
      def_t.gb_colnos[i] = static_cast<uint8_t>(colno_from_name(t,def.gb_col_names(i)));
    }
    def_t.num_cols = def.gb_col_nos_size();
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

  table_t *in = p->GetTable(request->tid().tableid());
  groupby_def_t gbd = make_groupby_def_t(in, request->def());
  table_t *out;
  if (request->in_sgx()) {
    out = aggregate_sgx(in, &gbd);
  } else {
    out = aggregate(in, &gbd);
  }
  auto tid = response->mutable_tid();
  tid->set_hostnum(p->HostNum());
  tid->set_tableid(p->AddTable(out));
  LOG(INFO) << "Success";
  for (int i = 0; i < out->num_tuples; i++) {
    print_tuple_log(i, get_tuple(i, out));
  }
  LOG(INFO) << out->num_tuples;
  return ::grpc::Status::OK;
}
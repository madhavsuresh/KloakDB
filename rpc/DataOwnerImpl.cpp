//
// Created by madhav on 10/1/18.
//

#include "DataOwnerImpl.h"
#include "../Aggregate.h"
#include "../Filter.h"
#include "../HashJoin.h"
#include "../Logger.h"
#include "../Repartition.h"
#include "../Sort.h"
#include "../pqxx_compat.h"
#include "DataOwnerPrivate.h"

DataOwnerImpl::DataOwnerImpl(DataOwnerPrivate *p) { this->p = p; }

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

      // TODO(madhavsuresh): refactor this code block out
      t->schema.num_fields = req.schema().num_fields();
      for (int i = 0; i < t->schema.num_fields; i++) {
        t->schema.fields[i].col_no = req.schema().field(i).col_no();
        strncpy(t->schema.fields[i].field_name,
                req.schema().field(i).field_name().c_str(), FIELD_NAME_LEN);
        switch (req.schema().field(i).field_type()) {
        case vaultdb::FieldDesc_FieldType_FIXEDCHAR: {
          // TODO(madhavsuresh): this should all be one enum.
          t->schema.fields[i].type = FIXEDCHAR;
          break;
        }
        case vaultdb::FieldDesc_FieldType_INT: {
          t->schema.fields[i].type = INT;
          break;
        }
        default: { throw; }
        }
      }
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

expr_t make_expr_t(const ::vaultdb::Expr &expr) {
  expr_t ex;
  ex.colno = expr.colno();
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
  expr_t ex = make_expr_t(request->expr());
  table_t *f = filter(p->GetTable(request->tid().tableid()), &ex);
  auto tid = response->mutable_tid();
  tid->set_hostnum(p->HostNum());
  tid->set_tableid(p->AddTable(f));
  LOG(INFO) << "Success";
  for (int i = 0; i < f->num_tuples; i++) {
    print_tuple_log(i, get_tuple(i, f));
  }
  return ::grpc::Status::OK;
}

sort_t make_sort_t(const ::vaultdb::SortDef sort) {
  sort_t s;
  s.colno = sort.colno();
  s.ascending = sort.ascending();
  return s;
}

::grpc::Status DataOwnerImpl::KSort(::grpc::ServerContext *context,
                                    const ::vaultdb::KSortRequest *request,
                                    ::vaultdb::KSortResponse *response) {
  sort_t s = make_sort_t(request->sortdef());

  table_t *sorted = sort(p->GetTable(request->tid().tableid()), &s);
  auto tid = response->mutable_tid();
  tid->set_hostnum(p->HostNum());
  tid->set_tableid(p->AddTable(sorted));
  LOG(INFO) << "Success";
  for (int i = 0; i < sorted->num_tuples; i++) {
    print_tuple_log(i, get_tuple(i, sorted));
  }
  return ::grpc::Status::OK;
}

join_def_t make_join_def_t(::vaultdb::JoinDef def) {
  join_def_t def_t;
  def_t.l_col = def.l_col();
  def_t.r_col = def.r_col();
  def_t.project_len = def.project_len();

  for (int i = 0; i < def.project_list_size(); i++) {
    auto &project = def.project_list(i);
    def_t.project_list[i].col_no = project.col_no();
    switch (project.side()) {
    case ::vaultdb::JoinColID_RelationSide_LEFT: {
      def_t.project_list[i].side = LEFT_RELATION;
      break;
    }
    case ::vaultdb::JoinColID_RelationSide_RIGHT: {
      def_t.project_list[i].side = RIGHT_RELATION;
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
  join_def_t def = make_join_def_t(request->def());
  table_t *out_join =
      hash_join(p->GetTable(request->left_tid().tableid()),
                p->GetTable(request->right_tid().tableid()), def);
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

groupby_def_t make_groupby_def_t(::vaultdb::GroupByDef def) {
  groupby_def_t def_t;

  switch (def.type()) {
  case ::vaultdb::GroupByDef_GroupByType_COUNT: {
    def_t.type = COUNT;
    break;
  }
  case ::vaultdb::GroupByDef_GroupByType_MINX: {
    def_t.type = MINX;
    def_t.colno = def.col_no();
    break;
  }
  case ::vaultdb::GroupByDef_GroupByType_AVG: {
    def_t.type = AVG;
    def_t.colno = def.col_no();
    for (int i = 0; i < def.gb_col_nos_size(); i++) {
      def_t.gb_colnos[i] = static_cast<uint8_t>(def.gb_col_nos(i));
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

  groupby_def_t gbd = make_groupby_def_t(request->def());
  table_t *out = aggregate(p->GetTable(request->tid().tableid()), &gbd);
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
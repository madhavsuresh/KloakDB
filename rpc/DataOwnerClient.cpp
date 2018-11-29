//
// Created by madhav on 10/1/18.
//

#include "DataOwnerClient.h"
#include "DataOwnerPrivate.h"
#include "logger/Logger.h"
#include <g3log/g3log.hpp>
#include "logger/LoggerDefs.h"


void DataOwnerClient::Shutdown() {
  vaultdb::ShutDownRequest req;
  vaultdb::ShutDownResponse resp;
  grpc::ClientContext context;
  auto status = stub_->ShutDown(&context, req, &resp);
  DOCLIENT_LOG_STATUS(shut_down, status);
}

void DataOwnerClient::GetPeerHosts(std::map<int, std::string> numToHostsMap) {
  vaultdb::GetPeerHostsRequest req;
  vaultdb::GetPeerHostsResponse resp;
  grpc::ClientContext context;

  std::map<int, std::string>::iterator it;
  for (it = numToHostsMap.begin(); it != numToHostsMap.end(); it++) {
    vaultdb::PeerHostInfo *p = req.add_hostnames();
    p->set_hostnum(it->first);
    p->set_hostname(it->second);
  }

  auto status = stub_->GetPeerHosts(&context, req, &resp);
  DOCLIENT_LOG_STATUS(get_peer_hosts, status);
}

std::vector<std::shared_ptr<const ::vaultdb::TableID>>
DataOwnerClient::RepartitionStepTwo(
    std::vector<std::shared_ptr<const ::vaultdb::TableID>> table_fragments) {
  vaultdb::RepartitionStepTwoRequest req;
  vaultdb::RepartitionStepTwoResponse resp;
  grpc::ClientContext context;

  std::vector<std::shared_ptr<const vaultdb::TableID>> vec;
  for (const auto &f : table_fragments) {
    ::vaultdb::TableID *tid = req.add_tablefragments();
    tid->CopyFrom(*f.get());
  }
  START_TIMER(repartition_step_two);
  auto status = stub_->RepartitionStepTwo(&context, req, &resp);
  END_AND_LOG_RPC_TIMER(repartition_step_two, host_name);
  DOCLIENT_LOG_STATUS(repartition_step_two, status);
  for (int i = 0; i < resp.remoterepartitionids_size(); i++) {
    auto tmp = std::make_shared<::vaultdb::TableID>();
    tmp.get()->CopyFrom(resp.remoterepartitionids(i));
    vec.emplace_back(tmp);
  }
  return vec;
}

std::vector<std::shared_ptr<const ::vaultdb::TableID>>
DataOwnerClient::RepartitionStepOne(
    std::shared_ptr<const ::vaultdb::TableID> tid) {
  vaultdb::RepartitionStepOneRequest req;
  vaultdb::RepartitionStepOneResponse resp;
  grpc::ClientContext context;

  auto t = req.mutable_tableid();
  t->set_hostnum(tid.get()->hostnum());
  t->set_tableid(tid.get()->tableid());

  START_TIMER(repartition_step_one);
  auto status = stub_->RepartitionStepOne(&context, req, &resp);
  END_AND_LOG_RPC_TIMER(repartition_step_one, host_name);
  DOCLIENT_LOG_STATUS(repartition_step_one, status);
  std::vector<std::shared_ptr<const vaultdb::TableID>> vec;
  for (int i = 0; i < resp.remoterepartitionids_size(); i++) {
    auto tmp = std::make_shared<vaultdb::TableID>();
    tmp.get()->CopyFrom(resp.remoterepartitionids(i));
    vec.emplace_back(tmp);
  }
  return vec;
}

std::shared_ptr<const ::vaultdb::TableID>
DataOwnerClient::DBMSQuery(std::string dbname, std::string query) {
  vaultdb::DBMSQueryRequest req;
  vaultdb::DBMSQueryResponse resp;
  grpc::ClientContext context;

  req.set_dbname(dbname);
  req.set_query(query);

  START_TIMER(dbms_rpc);
  auto status = stub_->DBMSQuery(&context, req, &resp);
  END_AND_LOG_RPC_TIMER(dbms_rpc, host_name);
  DOCLIENT_LOG_STATUS(dbms_query, status);
  auto tmp = std::make_shared<vaultdb::TableID>();
  tmp.get()->CopyFrom(resp.tableid());
  return tmp;
}

void table_schema_to_proto_schema(table_t *t, vaultdb::Schema *s) {
  s->set_num_fields(t->schema.num_fields);
  for (int i = 0; i < t->schema.num_fields; i++) {
    vaultdb::FieldDesc *fd = s->add_field();
    fd->set_field_name(t->schema.fields[i].field_name, FIELD_NAME_LEN);
    fd->set_col_no(t->schema.fields[i].col_no);
    switch (t->schema.fields[i].type) {
    case FIXEDCHAR: {
      fd->set_field_type(vaultdb::FieldDesc_FieldType_FIXEDCHAR);
      break;
    }
    case INT: {
      fd->set_field_type(vaultdb::FieldDesc_FieldType_INT);
      break;
    }
    case DOUBLE: {
      fd->set_field_type(vaultdb::FieldDesc_FieldType_DOUBLE);
      break;
    }
    case TIMESTAMP: {
      fd->set_field_type(vaultdb::FieldDesc_FieldType_TIMESTAMP);
      break;
    }
    case UNSUPPORTED: {
      throw;
    }
    default: { throw; }
    }
  }
}

std::shared_ptr<const ::vaultdb::TableID> DataOwnerClient::CoalesceTables(
    std::vector<std::shared_ptr<const ::vaultdb::TableID>> &tables) {
  vaultdb::CoaleseTablesRequest req;
  vaultdb::CoaleseTablesResponse resp;
  grpc::ClientContext context;

  for (auto t : tables) {
    auto tf = req.add_tablefragments();
    tf->CopyFrom(*t.get());
  }
  START_TIMER(coalesce_rpc);
  auto status = stub_->CoalesceTables(&context, req, &resp);
  END_AND_LOG_RPC_TIMER(coalesce_rpc, host_name);
  DOCLIENT_LOG_STATUS(coalesce_tables, status);
  auto ret = std::make_shared<::vaultdb::TableID>();
  ret.get()->CopyFrom(resp.id());
  return ret;
}

std::shared_ptr<const ::vaultdb::TableID>
DataOwnerClient::GenZip(std::shared_ptr<const ::vaultdb::TableID> gen_map,
                        std::shared_ptr<const ::vaultdb::TableID> scan_table,
                        std::string col_name) {
  ::vaultdb::GeneralizeZipRequest req;
  ::vaultdb::GeneralizeZipResponse resp;
  ::grpc::ClientContext context;

  req.set_colname(col_name);
  auto gt = req.mutable_gentableid();
  gt->set_tableid(gen_map.get()->tableid());
  gt->set_hostnum(gen_map.get()->hostnum());
  auto st = req.mutable_scantableid();
  st->set_tableid(scan_table.get()->tableid());
  st->set_hostnum(scan_table.get()->hostnum());
  START_TIMER(genzip_rpc);
  auto status = stub_->GeneralizeZip(&context, req, &resp);
  END_AND_LOG_RPC_TIMER(genzip_rpc, host_name);
  DOCLIENT_LOG_STATUS(generalize_zip, status);
  auto ret = std::make_shared<::vaultdb::TableID>();
  ret.get()->CopyFrom(resp.generalizedscantable());
  return ret;
}

std::shared_ptr<const ::vaultdb::TableID>
DataOwnerClient::Filter(std::shared_ptr<const ::vaultdb::TableID> tid,
                        ::vaultdb::Expr expr, bool in_sgx) {
  ::vaultdb::KFilterRequest req;
  ::vaultdb::KFilterResponse resp;
  ::grpc::ClientContext context;

  req.mutable_expr()->CopyFrom(expr);
  req.set_in_sgx(in_sgx);
  auto t = req.mutable_tid();
  t->set_hostnum(tid.get()->hostnum());
  t->set_tableid(tid.get()->tableid());
  START_TIMER(filter_rpc);
  auto status = stub_->KFilter(&context, req, &resp);
  END_AND_LOG_RPC_TIMER(filter_rpc, host_name);
  DOCLIENT_LOG_STATUS(k_filter, status);
  auto ret = std::make_shared<::vaultdb::TableID>();
  ret.get()->CopyFrom(resp.tid());
  return ret;
}

void DataOwnerClient::FreeTable(
    std::shared_ptr<const ::vaultdb::TableID> id_ptr) {

  ::vaultdb::FreeTableRequest req;
  ::vaultdb::FreeTableResponse resp;
  ::grpc::ClientContext context;
  auto t = req.mutable_tid();
  t->set_hostnum(id_ptr.get()->hostnum());
  t->set_tableid(id_ptr.get()->tableid());
  START_TIMER(free_table_rpc);
  auto status = stub_->FreeTable(&context, req, &resp);
  END_AND_LOG_RPC_TIMER(free_table_rpc, host_name);
  DOCLIENT_LOG_STATUS(client_free_table, status);
}

table_t *
DataOwnerClient::GetTable(std::shared_ptr<const ::vaultdb::TableID> id_ptr) {
  // TODO(madhavsuresh): this is copy pasted code, this block should be
  // refactored out.
  ::vaultdb::GetTableRequest req;
  ::vaultdb::GetTableResponse resp;
  ::grpc::ClientContext context;
  table_t *t = nullptr;
  auto id = id_ptr.get();

  req.mutable_id()->CopyFrom(*id);
  START_TIMER(get_table_rpc);
  std::unique_ptr<::grpc::ClientReader<::vaultdb::GetTableResponse>> reader(
      stub_->GetTable(&context, req));
  while (reader->Read(&resp)) {
    if (resp.is_header()) {
      t = allocate_table(resp.num_tuple_pages());
      t->num_tuples = resp.num_tuples();
      t->size_of_tuple = resp.size_of_tuple();
      t->num_tuple_pages = resp.num_tuple_pages();

      proto_schema_to_table_schema(t, resp.schema());
    } else {
      t->tuple_pages[resp.page_no()] = (tuple_page_t *)malloc(PAGE_SIZE);
      memcpy(t->tuple_pages[resp.page_no()], resp.page().c_str(), PAGE_SIZE);
    }
  }
  auto status = reader->Finish();
  END_AND_LOG_RPC_TIMER(get_table_rpc, host_name);
  DOCLIENT_LOG_STATUS(get_table, status);
  if (t == nullptr) {
    throw;
  }
  return t;
}

// TODO(madhavsuresh): refactor this to return
// tableid
int DataOwnerClient::SendTable(table_t *t) {
  ::vaultdb::SendTableResponse resp;
  ::grpc::ClientContext context;

  std::unique_ptr<::grpc::ClientWriter<::vaultdb::SendTableRequest>> writer(
      stub_->SendTable(&context, &resp));

  ::vaultdb::SendTableRequest header;
  header.set_is_header(true);
  header.set_num_tuples(t->num_tuples);
  header.set_num_tuple_pages(t->num_tuple_pages);
  header.set_size_of_tuple(t->size_of_tuple);

  ::vaultdb::Schema *s = header.mutable_schema();
  table_schema_to_proto_schema(t, s);
  START_TIMER(send_table_rpc);
  writer->Write(header);

  ::vaultdb::SendTableRequest pages;
  for (int i = 0; i < t->num_tuple_pages; i++) {
    pages.set_is_header(false);
    pages.set_page_no(i);
    pages.set_page((char *)t->tuple_pages[i], PAGE_SIZE);
    writer->Write(pages);
  }
  writer->WritesDone();
  ::grpc::Status status = writer->Finish();
  END_AND_LOG_RPC_TIMER(send_table_rpc, host_name);

  DOCLIENT_LOG_STATUS(send_table, status);
  return resp.tableid();
}

std::shared_ptr<const ::vaultdb::TableID>
DataOwnerClient::Sort(std::shared_ptr<const ::vaultdb::TableID> tid,
                      ::vaultdb::SortDef sort, bool in_sgx) {
  ::vaultdb::KSortRequest req;
  ::vaultdb::KSortResponse resp;
  ::grpc::ClientContext context;
  req.mutable_sortdef()->CopyFrom(sort);
  auto t = req.mutable_tid();
  req.set_in_sgx(in_sgx);
  t->set_hostnum(tid.get()->hostnum());
  t->set_tableid(tid.get()->tableid());
  START_TIMER(sort_rpc);
  auto status = stub_->KSort(&context, req, &resp);
  END_AND_LOG_RPC_TIMER(sort_rpc, host_name);
  DOCLIENT_LOG_STATUS(client_sort, status);
  auto ret = std::make_shared<::vaultdb::TableID>();
  ret.get()->CopyFrom(resp.tid());
  return ret;
}

std::shared_ptr<const ::vaultdb::TableID>
DataOwnerClient::Join(std::shared_ptr<const ::vaultdb::TableID> left_tid,
                      std::shared_ptr<const ::vaultdb::TableID> right_tid,
                      ::vaultdb::JoinDef join, bool in_sgx) {
  ::vaultdb::KJoinRequest req;
  ::vaultdb::KJoinResponse resp;
  ::grpc::ClientContext context;
  req.mutable_def()->CopyFrom(join);
  req.set_in_sgx(in_sgx);
  auto l = req.mutable_left_tid();
  l->set_hostnum(left_tid.get()->hostnum());
  l->set_tableid(left_tid.get()->tableid());

  auto r = req.mutable_right_tid();
  r->set_hostnum(right_tid.get()->hostnum());
  r->set_tableid(right_tid.get()->tableid());
  START_TIMER(join_rpc_call);
  auto status = stub_->KJoin(&context, req, &resp);
  END_AND_LOG_RPC_TIMER(join_rpc_call, host_name);
  DOCLIENT_LOG_STATUS(client_join, status);
  auto ret = std::make_shared<::vaultdb::TableID>();
  ret.get()->CopyFrom(resp.tid());
  return ret;
}

std::shared_ptr<const ::vaultdb::TableID>
DataOwnerClient::Aggregate(std::shared_ptr<const ::vaultdb::TableID> tid,
                           ::vaultdb::GroupByDef groupby, bool in_sgx) {
  ::vaultdb::KAggregateRequest req;
  ::vaultdb::KAggregateResponse resp;
  ::grpc::ClientContext context;
  req.mutable_def()->CopyFrom(groupby);
  req.set_in_sgx(in_sgx);
  auto t = req.mutable_tid();
  t->set_hostnum(tid.get()->hostnum());
  t->set_tableid(tid.get()->tableid());
  START_TIMER(aggregate_rpc);
  auto status = stub_->KAggregate(&context, req, &resp);
  END_AND_LOG_RPC_TIMER(aggregate_rpc, host_name);
  DOCLIENT_LOG_STATUS(client_aggregate, status);
  auto ret = std::make_shared<::vaultdb::TableID>();
  ret.get()->CopyFrom(resp.tid());
  return ret;
}

std::shared_ptr<const ::vaultdb::TableID>
        DataOwnerClient::MakeObli(std::shared_ptr<const ::vaultdb::TableID> tid, std::string col_name) {
  ::vaultdb::MakeObliRequest req;
  ::vaultdb::MakeObliResponse resp;
  ::grpc::ClientContext context;

  req.set_col_name(col_name);
  auto t = req.mutable_tid();
  t->set_hostnum(tid.get()->hostnum());
  t->set_tableid(tid.get()->tableid());
  START_TIMER(makeobli_rpc);
  auto status = stub_->MakeObli(&context, req, &resp);
  END_TIMER(makeobli_rpc);
  DOCLIENT_LOG_STATUS(makeobli, status);
  auto ret = std::make_shared<::vaultdb::TableID>();
  ret.get()->CopyFrom(resp.tid());
  return ret;
}

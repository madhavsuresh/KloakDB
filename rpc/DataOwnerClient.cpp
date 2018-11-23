//
// Created by madhav on 10/1/18.
//

#include "DataOwnerClient.h"
#include "DataOwnerPrivate.h"
#include <g3log/g3log.hpp>

void DataOwnerClient::Shutdown() {
  vaultdb::ShutDownRequest req;
  vaultdb::ShutDownResponse resp;
  grpc::ClientContext context;
  auto status = stub_->ShutDown(&context, req, &resp);
  if (status.ok()) {

  } else {

  }
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
  if (status.ok()) {
    LOG(INFO) << "SUCCESS:->[" << host_num << "]";
  } else {
    LOG(INFO) << "FAIL:->[" << host_num << "]";
  }
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
  auto status = stub_->RepartitionStepTwo(&context, req, &resp);
  if (status.ok()) {
    LOG(INFO) << "SUCCESS:->[" << host_num << "], RepartitionStepTwo";
  } else {
    LOG(INFO) << "FAIL:->[" << host_num << "]";
  }
  for (int i = 0; i < resp.remoterepartitionids_size(); i++) {
    // TODO(madhavsuresh): figure out how CPP memory managment works
    auto tmp = std::make_shared<::vaultdb::TableID>();
    tmp.get()->CopyFrom(resp.remoterepartitionids(i));
    vec.emplace_back(tmp);
  }

  return vec;
}

std::vector<std::shared_ptr<const ::vaultdb::TableID>>
DataOwnerClient::RepartitionStepOne(std::shared_ptr<const ::vaultdb::TableID> tid) {
  vaultdb::RepartitionStepOneRequest req;
  vaultdb::RepartitionStepOneResponse resp;
  grpc::ClientContext context;

  auto t = req.mutable_tableid();
  t->set_hostnum(tid.get()->hostnum());
  t->set_tableid(tid.get()->tableid());

  auto status = stub_->RepartitionStepOne(&context, req, &resp);
  if (status.ok()) {
    LOG(INFO) << "SUCCESS:->[" << host_num << "], Repartition at tableID: ["
              << tid.get()->tableid() << "]";
  } else {
    LOG(INFO) << "FAIL:->[" << host_num << "]";
  }
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

  auto status = stub_->DBMSQuery(&context, req, &resp);
  if (status.ok()) {
    LOG(INFO) << "SUCCESS:->[" << host_num << "]"
              << "Query: [" << query << "]";
    auto tmp = std::make_shared<vaultdb::TableID>();
    tmp.get()->CopyFrom(resp.tableid());
    return tmp;
  } else {
    LOG(INFO) << "FAIL:->[" << host_num << "]";
    std::cerr << status.error_code() << ": " << status.error_message()
              << std::endl;
    throw;
  }
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
    case UNSUPPORTED : {
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
  auto status = stub_->CoalesceTables(&context, req, &resp);
  if (status.ok()) {
    LOG(INFO) << "SUCCESS:->[" << host_num << "]";
    auto ret = std::make_shared<::vaultdb::TableID>();
    ret.get()->CopyFrom(resp.id());
    return ret;
  } else {
    LOG(INFO) << "FAIL:->[" << host_num << "]";
    std::cerr << status.error_code() << ": " << status.error_message()
              << std::endl;
    throw;
  }
}

std::shared_ptr<const ::vaultdb::TableID>
        DataOwnerClient::GenZip(std::shared_ptr<const ::vaultdb::TableID> gen_map, std::shared_ptr<const ::vaultdb::TableID> scan_table) {
  ::vaultdb::GeneralizeZipRequest req;
  ::vaultdb::GeneralizeZipResponse resp;
  ::grpc::ClientContext context;

  auto gt = req.mutable_gentableid();
  gt->set_tableid(gen_map.get()->tableid());
  gt->set_hostnum(gen_map.get()->hostnum());
  auto st = req.mutable_scantableid();
  st->set_tableid(scan_table.get()->tableid());
  st->set_hostnum(scan_table.get()->hostnum());
  auto status = stub_->GeneralizeZip(&context, req, &resp);
  if (status.ok()) {
    LOG(INFO) << "SUCCESS:->[" << host_num << "]";
    auto ret = std::make_shared<::vaultdb::TableID>();
    ret.get()->CopyFrom(resp.generalizedscantable());
    return ret;
  } else {
    LOG(INFO) << "FAIL:->[" << host_num << "]";
    std::cerr << status.error_code() << ": " << status.error_message()

              << std::endl;
    throw;
  }
}

std::shared_ptr<const ::vaultdb::TableID>
DataOwnerClient::Filter(std::shared_ptr<const ::vaultdb::TableID> tid,
                        ::vaultdb::Expr expr) {
  ::vaultdb::KFilterRequest req;
  ::vaultdb::KFilterResponse resp;
  ::grpc::ClientContext context;

  req.mutable_expr()->CopyFrom(expr);
  auto t = req.mutable_tid();
  t->set_hostnum(tid.get()->hostnum());
  t->set_tableid(tid.get()->tableid());
  auto status = stub_->KFilter(&context, req, &resp);
  if (status.ok()) {
    LOG(INFO) << "SUCCESS:->[" << host_num << "]";
    auto ret = std::make_shared<::vaultdb::TableID>();
    ret.get()->CopyFrom(resp.tid());
    return ret;
  } else {
    LOG(INFO) << "FAIL:->[" << host_num << "]";
    std::cerr << status.error_code() << ": " << status.error_message()

              << std::endl;
    throw;
  }
}

table_t * DataOwnerClient::GetTable(std::shared_ptr<const ::vaultdb::TableID> id_ptr) {
  // TODO(madhavsuresh): this is copy pasted code, this block should be refactored out.
  ::vaultdb::GetTableRequest req;
  ::vaultdb::GetTableResponse resp;
  ::grpc::ClientContext context;
  table_t * t;
  auto id = id_ptr.get();

  req.mutable_id()->CopyFrom(*id);
  std::unique_ptr<::grpc::ClientReader<::vaultdb::GetTableResponse>> reader(stub_->GetTable(&context, req));
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
  if (status.ok()) {
    LOG(INFO) << "SUCCESS:->[" << host_num << "]";
  } else {
    LOG(INFO) << "FAILURE";
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

  if (status.ok()) {
    LOG(INFO) << "SUCCESS:->[" << host_num << "], num_tuples:[" << t->num_tuples
              << "], num_pages: [" << t->num_tuple_pages << "]";
    return resp.tableid();
  } else {
    LOG(INFO) << "FAIL:->[" << host_num << "]";
    std::cerr << status.error_code() << ": " << status.error_message()
              << std::endl;
    throw;
  }
}

std::shared_ptr<const ::vaultdb::TableID>
DataOwnerClient::Sort(std::shared_ptr<const ::vaultdb::TableID> tid,
                      ::vaultdb::SortDef sort) {
  ::vaultdb::KSortRequest req;
  ::vaultdb::KSortResponse resp;
  ::grpc::ClientContext context;
  req.mutable_sortdef()->CopyFrom(sort);
  auto t = req.mutable_tid();
  t->set_hostnum(tid.get()->hostnum());
  t->set_tableid(tid.get()->tableid());
  auto status = stub_->KSort(&context, req, &resp);
  if (status.ok()) {
    LOG(INFO) << "SUCCESS:->[" << host_num << "]";
    auto ret = std::make_shared<::vaultdb::TableID>();
    ret.get()->CopyFrom(resp.tid());
    return ret;
  } else {
    LOG(INFO) << "FAIL:->[" << host_num << "]";
    std::cerr << status.error_code() << ": " << status.error_message()
              << std::endl;
    throw;
  }
}

std::shared_ptr<const ::vaultdb::TableID>
DataOwnerClient::Join(std::shared_ptr<const ::vaultdb::TableID> left_tid,
                      std::shared_ptr<const ::vaultdb::TableID> right_tid,
                      ::vaultdb::JoinDef join) {
  ::vaultdb::KJoinRequest req;
  ::vaultdb::KJoinResponse resp;
  ::grpc::ClientContext context;
  req.mutable_def()->CopyFrom(join);
  auto l = req.mutable_left_tid();
  l->set_hostnum(left_tid.get()->hostnum());
  l->set_tableid(left_tid.get()->tableid());

  auto r = req.mutable_right_tid();
  r->set_hostnum(right_tid.get()->hostnum());
  r->set_tableid(right_tid.get()->tableid());
  auto status = stub_->KJoin(&context, req, &resp);
  if (status.ok()) {
    LOG(INFO) << "SUCCESS:->[" << host_num << "]";
    auto ret = std::make_shared<::vaultdb::TableID>();
    ret.get()->CopyFrom(resp.tid());
    return ret;
  } else {
    LOG(INFO) << "FAIL:->[" << host_num << "]";
    std::cerr << status.error_code() << ": " << status.error_message()
              << std::endl;
    throw;
  }
}

std::shared_ptr<const ::vaultdb::TableID>
DataOwnerClient::Aggregate(std::shared_ptr<const ::vaultdb::TableID> tid,
                           ::vaultdb::GroupByDef groupby) {
  ::vaultdb::KAggregateRequest req;
  ::vaultdb::KAggregateResponse resp;
  ::grpc::ClientContext context;
  req.mutable_def()->CopyFrom(groupby);
  auto t = req.mutable_tid();
  t->set_hostnum(tid.get()->hostnum());
  t->set_tableid(tid.get()->tableid());
  auto status = stub_->KAggregate(&context, req, &resp);
  if (status.ok()) {
    LOG(INFO) << "SUCCESS:->[" << host_num << "]";
    auto ret = std::make_shared<::vaultdb::TableID>();
    ret.get()->CopyFrom(resp.tid());
    return ret;
  } else {
    LOG(INFO) << "FAIL:->[" << host_num << "]";
    std::cerr << status.error_code() << ": " << status.error_message()
              << std::endl;
    throw;
  }
}

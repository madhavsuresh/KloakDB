//
// Created by madhav on 9/17/18.
//
#include "../postgres_client.h"
#include "HBGeneralize.h"
#include "vaultdb.grpc.pb.h"
#include <grpcpp/grpcpp.h>

class VaultDBClient {
public:
  VaultDBClient(std::shared_ptr<grpc::Channel> channel)
      : stub_(vaultdb::VaultDBOperators::NewStub(channel)) {}

  int DBQuery() {
    vaultdb::DBQueryRequest req;
    vaultdb::DBQueryResponse resp;
    req.set_dbname("dbname=test");
    req.set_query_string("SELECT * FROM rpc_test;");

    grpc::ClientContext context;
    auto status = stub_->DBQuery(&context, req, &resp);
    if (status.ok()) {

    } else {
      std::cerr << status.error_code() << ": " << status.error_message()
                << std::endl;
      throw;
    }
  }

  // TODO(madhavsuresh): write tests for this
  table_t *GetTable(std::string dbname, std::string query) {
    table_t *t;
    grpc::ClientContext context;
    vaultdb::TableQueryRequest req;
    vaultdb::TableQueryResponse resp;

    req.set_dbname(dbname);
    req.set_query(query);

    std::unique_ptr<grpc::ClientReader<vaultdb::TableQueryResponse>> reader(
        stub_->GetTable(&context, req));

    while (reader->Read(&resp)) {

      if (resp.is_header()) {
        t = allocate_table(resp.num_tuple_pages());

        t->num_tuples = resp.num_tuples();
        t->size_of_tuple = resp.size_of_tuple();
        t->num_tuple_pages = resp.num_tuple_pages();

        // TODO(madhavsuresh): refactor this code block out
        t->schema.num_fields = resp.schema().num_fields();
        for (int i = 0; i < t->schema.num_fields; i++) {
          t->schema.fields[i].col_no = resp.schema().field(i).col_no();
          strncpy(t->schema.fields[i].field_name,
                  resp.schema().field(i).field_name().c_str(), FIELD_NAME_LEN);
          switch (resp.schema().field(i).field_type()) {
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
        t->tuple_pages[resp.page_no()] = (tuple_page_t *)malloc(PAGE_SIZE);
        memcpy(t->tuple_pages[resp.page_no()], resp.page().c_str(), PAGE_SIZE);
      }
    }
    return t;
  }

  int StreamTable() {
    grpc::ClientContext context;
    vaultdb::TableStream t;
    vaultdb::TableRequest tr;
    std::unique_ptr<grpc::ClientReader<vaultdb::TableStream>> reader(
        stub_->GetTableStream(&context, tr));
    while (reader->Read(&t)) {
      printf("STREAMING\n: ");
      auto tp = (tuple_page_t *)t.page().c_str();
      printf("%d", t.page().length());
    }
  }

private:
  std::unique_ptr<vaultdb::VaultDBOperators::Stub> stub_;
};

void Generalize(std::vector<TableStatistics> allStats) {
  int num_hosts = allStats.size();
  for (int i = 0; i < num_hosts; i++) {
    for (int j = 0; j < num_hosts; j++) {
      std::map<int, int> newMap;
      if (j == i) {
        continue;
      }
      std::map<int, int> currMap = allStats.at(j).GetMap();
      for (std::map<int, int>::iterator it = currMap.begin();
           it != currMap.end(); ++it) {
        if (newMap.count(it->first) == 1) {
          newMap[it->first] += it->second;
        } else {
          newMap[it->first] = it->second;
        }
      }
    }
  }
}

void generalize() {
  std::vector<TableStatistics> allStats;
  VaultDBClient client(
      grpc::CreateChannel("0.0.0.0:50051", grpc::InsecureChannelCredentials()));
  std::string db1 = "dbname=test";
  std::string q1 = "SELECT floor, COUNT(*) from rpc_test group by floor;";
  std::string q2 = "SELECT floor, COUNT(*) from rpc_test1 group by floor;";
  table_t *t1 = client.GetTable(db1, q1);
  table_t *t2 = client.GetTable(db1, q2);
  TableStatistics ts;
  ts.IngestAllocatedTable(t1);
  TableStatistics ts2;
  ts2.IngestAllocatedTable(t2);
  allStats.push_back(ts);
  allStats.push_back(ts2);
  Generalize(allStats);
}

int main() { generalize(); }

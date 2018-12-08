//
// Created by madhav on 10/30/18.
//
#include "Logger.h"

void print_tuple(tuple_t *t) { std::cout << tuple_string(t); }

void print_tuple_log(int ii, tuple_t *t) {
  std::string output;
  if (t->is_dummy) {
    LOGF(INFO, "tuple is dummy");
    return;
  }
  output += std::to_string(ii) + "{";
  if (t->num_fields == 0) {
    LOGF(INFO, "NUM _FIELDS IS ZERO!!");
  }
  for (int i = 0; i < t->num_fields; i++) {
    switch (t->field_list[i].type) {
    case FIXEDCHAR: {
      output += std::to_string(i) + ":|" +
                std::string(t->field_list[i].f.fixed_char_field.val) + "|";
      break;
    }
    case INT: {
      output += std::to_string(i) + ":|" +
                std::to_string(t->field_list[i].f.int_field.val) + "|";
      // snprintf(output+offset, 8, "%d:|%d|",i,
      // t->field_list[i].f.int_field.val);  offset+=8;
      break;
    }
    case UNSUPPORTED: {
      throw;
    }
    }
  }
  LOGF(INFO, "%s}", output.c_str());
}

void RemoteSink::ReceiveLogMessage(g3::LogMessageMover logEntry) {
  send_log_to_server(logEntry.get().toString());
}

void RemoteSink::send_log_to_server(std::string log) {
  ::log_server::LogRequest req;
  ::log_server::LogReply resp;
  ::grpc::ClientContext context;
  req.set_logmessage("(" + host_short + ")" + log);
  auto status = stub_->Log(&context, req, &resp);
  if (!status.ok()) {
    printf("%s\n", log.c_str());
    fflush(0);
    //throw;
  }
}

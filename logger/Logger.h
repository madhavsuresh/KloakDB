//
// Created by madhav on 10/30/18.
//

#ifndef PROJECT_LOGGER_H
#define PROJECT_LOGGER_H

#include "data/postgres_client.h"
#include <iostream>
#include "logger.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <g3log/g3log.hpp>

void print_tuple_log(int ii, tuple_t *t);
void print_tuple(tuple_t *t);
void send_log_to_server(std::string);




struct RemoteSink {
    std::unique_ptr<::log_server::Logs::Stub> stub_;
    std::string host_short;

    RemoteSink(std::string host_short, std::shared_ptr<grpc::Channel> channel)
    :host_short(host_short), stub_(::log_server::Logs::NewStub(channel)) {}
    void ReceiveLogMessage(g3::LogMessageMover logEntry);
    void send_log_to_server(std::string log);
};
#endif // PROJECT_LOGGER_H

//
// Created by madhav on 11/24/18.
//

#ifndef PROJECT_LOGGERDEFS_H
#define PROJECT_LOGGERDEFS_H
#include <g3log/g3log.hpp>

const LEVELS STATS{INFO.value - 1, "STATS"};
const LEVELS RPC_STATS{INFO.value + 1, "RPC-STATS"};
const LEVELS DO_CLIE{INFO.value + 40, "DO-CLIE"};
const LEVELS DO_CLIE_FATAL{INFO.value + 41, "DO-CLIE-FATAL"};
const LEVELS DO_IMPL{INFO.value + 2, "DO-IMPL"};
const LEVELS SGX_STATS{INFO.value + 10, "SGX-STATS"};
const LEVELS SGX {INFO.value + 11, "SGX"};
const LEVELS SGX_FATAL {INFO.value + 12, "SGX-FATAL"};
const LEVELS OP {INFO.value + 30, "OP"};

#define START_TIMER(timer_name)                                                \
  auto start_##timer_name = std::chrono::high_resolution_clock::now()
#define END_TIMER(timer_name)                                                  \
  auto end_##timer_name = std::chrono::high_resolution_clock::now()
#define LOG_TIMER(timer_name)                                                  \
  std::chrono::duration<double> elapsed_##timer_name =                         \
      end_##timer_name - start_##timer_name;                                   \
  LOG(STATS) << "TIMER " << #timer_name << ": ["                               \
             << elapsed_##timer_name.count() << "s]"

#define LOG_SGX_TIMER(timer_name) \
  std::chrono::duration<double> elapsed_##timer_name =                         \
      end_##timer_name - start_##timer_name;                                   \
  LOG(SGX_STATS) << "TIMER " << #timer_name << ": ["                               \
             << elapsed_##timer_name.count() << "s]"

#define LOG_RPC_TIMER(timer_name, hostname)                                                  \
  std::chrono::duration<double> elapsed_##timer_name =                         \
      end_##timer_name - start_##timer_name;                                   \
  LOG(RPC_STATS) << "[" << hostname << "] TIMER " << #timer_name << ": ["                               \
             << elapsed_##timer_name.count() << "s]"

#define END_AND_LOG_TIMER(timer_name)                                          \
  do {                                                                         \
    END_TIMER(timer_name);                                                     \
    LOG_TIMER(timer_name);                                                     \
  } while (0)

#define END_AND_LOG_RPC_TIMER(timer_name, hostname)                                          \
  do {                                                                         \
    END_TIMER(timer_name);                                                     \
    LOG_RPC_TIMER(timer_name, hostname);                                                     \
  } while (0)

#define END_AND_LOG_SGX_TIMER(timer_name)                                          \
  do {                                                                         \
    END_TIMER(timer_name);                                                     \
    LOG_SGX_TIMER(timer_name);                                                     \
  } while (0)
#define DOCLIENT_LOG_STATUS(CLIENTCALL, status)                                \
  do {                                                                         \
    if (status.ok()) {                                                         \
      DOCLIENT_LOG_OK(CLIENTCALL);                                             \
    } else {                                                                   \
      DOCLIENT_LOG_FAIL(CLIENTCALL);                                           \
      throw;                                                                   \
    }                                                                          \
  } while (0)

#define DOCLIENT_LOG_OK(CLIENTCALL)                                            \
  LOG(DO_CLIE) << "[" << host_name << "]" << #CLIENTCALL << " OK"
#define DOCLIENT_LOG_FAIL(CLIENTCALL)                                          \
  LOG(DO_CLIE_FATAL) << "[" << host_name << "]" << #CLIENTCALL << "Error : ["  \
                     << status.error_message() << "],            \
      Error Details "             \
                     << status.error_details()

#endif // PROJECT_LOGGERDEFS_H

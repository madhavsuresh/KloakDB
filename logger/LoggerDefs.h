//
// Created by madhav on 11/24/18.
//

#ifndef PROJECT_LOGGERDEFS_H
#define PROJECT_LOGGERDEFS_H
#include <g3log/g3log.hpp>
#ifndef LOGDEFS
#define LOGDEFS
LEVELS IMPL{INFO.value + 1, "RPC-IMPL"};
LEVELS CLIENT{IMPL.value + 1, "RPC-CLIENT"};
LEVELS STATS{CLIENT.value + 1, "STATS"};

#define START_TIMER(timer_name)                                                \
  auto start_##timer_name = std::chrono::high_resolution_clock::now()
#define END_TIMER(timer_name)                                                  \
  auto end_##timer_name = std::chrono::high_resolution_clock::now()
#define LOG_TIMER(timer_name)                                                  \
  std::chrono::duration<double> elapsed_##timer_name =                         \
      end_##timer_name - start_##timer_name;                                   \
  LOG(STATS) << "TIMER " << #timer_name << ": [" << elapsed_##timer_name.count() << "s]"
#endif // LOGDEFS

#endif // PROJECT_LOGGERDEFS_H

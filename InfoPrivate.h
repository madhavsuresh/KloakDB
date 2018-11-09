//
// Created by madhav on 10/1/18.
//

#ifndef PROJECT_PRIVATE_H
#define PROJECT_PRIVATE_H

#include <string>

class InfoPrivate {
private:
  std::string hostName;

public:
  InfoPrivate(std::string hostname);
  std::string HostName();
  void SetHostName(std::string hostname);
};

#endif // PROJECT_PRIVATE_H

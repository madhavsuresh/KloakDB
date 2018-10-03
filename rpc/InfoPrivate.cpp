//
// Created by madhav on 10/1/18.
//

#include "InfoPrivate.h"

InfoPrivate::InfoPrivate(std::string hostname) {
    this->hostName = hostname;
}
std::string InfoPrivate::HostName() {
    return this->hostName;
}

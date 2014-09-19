#include "util.h"

std::vector<std::string> util::split(const std::string &s, char delim) {
    std::vector<std::string> elems;
   std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}
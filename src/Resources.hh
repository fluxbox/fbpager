#ifndef RESOURCES_HH
#define RESOURCES_HH

#include "FbPager.hh"

#include "FbTk/Resource.hh"
#include <string>
#include <cstdio>
#include <cstring>

namespace FbTk {

template<>
void FbTk::Resource<bool>::
setFromString(char const *strval) {
    if (strcasecmp(strval, "true") == 0 ||
        strcasecmp(strval, "yes") == 0)
        *this = true;
    else
        *this = false;
}

template<>
std::string FbTk::Resource<bool>::
getString() {				
    return std::string(**this == true ? "true" : "false");
}

template <>
void FbTk::Resource<std::string>::setFromString(const char *str) {
    *(*this) = (str ?  str : "");
}

template <>
std::string FbTk::Resource<std::string>::getString() {
    return *(*this);
}

template <>
void FbTk::Resource<int>::setFromString(const char *str) {
    if (str == 0)
        return;
    sscanf(str, "%d", &(*(*this)));
}

template <>
std::string FbTk::Resource<int>::getString() {
    char buff[16];
    sprintf(buff, "%d", (*(*this)));
    return std::string(buff);
}



template <>
void FbTk::Resource<FbPager::FbPager::Alignment>::
setFromString(const char *str) {
    if (strcmp("TopToBottom", str) == 0)
        *(*this) = FbPager::FbPager::TOP_TO_BOTTOM;
    else
        *(*this) = FbPager::FbPager::LEFT_TO_RIGHT;
}

template <>
std::string FbTk::Resource<FbPager::FbPager::Alignment>::getString() {
    switch (*(*this)) {
    case FbPager::FbPager::LEFT_TO_RIGHT:
        return "LeftToRight";
    case FbPager::FbPager::TOP_TO_BOTTOM:
        return "TopToBottom";
    }
}


}
#endif // RESOURCES

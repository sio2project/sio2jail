#pragma once

#include "Exception.h"
#include "Preprocessor.h"

#include "logger/Logger.h"

#ifdef assert
#undef assert
#endif

#define assert(...) \
  CAT(assert_, VA_SIZE(__VA_ARGS__))(__VA_ARGS__)

#define assert_1(assertion) \
  if (!(assertion)) { \
    throw s2j::AssertionException( \
        "Assertion failed at " __FILE__ ":" + std::to_string(__LINE__) + " " #assertion); \
  }

#define assert_2(assertion, comment) \
  if (!(assertion)) { \
    throw s2j::AssertionException( \
        "Assertion " + std::string(comment) + " failed at " __FILE__ ":" + \
        std::to_string(__LINE__) + " " #assertion); \
  }

namespace s2j {

class AssertionException : public Exception {
public:
    AssertionException(const std::string& msg)
        : Exception(msg) {
        logger::error(msg);
    }
};

}

#pragma once

#include <array>
#include <string>

namespace s2j {
namespace app {
namespace args {

/**
 * A simple wrapper for memory sizes read from command line.
 */
class MemoryArgument {
    static const std::array<std::string, 4> sizes;

    size_t value_; // Size in bytes

public:
    MemoryArgument() : value_(0) {
    }

    MemoryArgument(size_t value) : value_(value) {
    }
    
    MemoryArgument& operator=(const std::string &str);

    size_t getValue() const {
        return value_;
    }

    operator size_t() const {
        return getValue();
    }
};



/**
 * A simple wrapper for times read from command line.
 */
class TimeArgument {
    static const std::array<std::string, 6> sizes;
    static const std::array<uint64_t, 6> multipliers;

    size_t value_; // Size in microseconds

public:
    TimeArgument() : value_(0) {
    }

    TimeArgument(size_t value) : value_(value) {
    }

    TimeArgument& operator=(const std::string &str);

    size_t getValue() const {
        return value_;
    }

    operator size_t() const {
        return getValue();
    }

};

/**
 * A simple wrapper for large numbers from command line.
 */
class AmountArgument {
    static const std::array<std::string, 4> sizes;

    size_t value_; // Size in microseconds

public:
    AmountArgument() : value_(0) {
    }

    AmountArgument(size_t value) : value_(value) {
    }

    AmountArgument& operator=(const std::string &str);

    size_t getValue() const {
        return value_;
    }

    operator size_t() const {
        return getValue();
    }

};

}
}
}
namespace TCLAP {
template<>
struct ArgTraits<::s2j::app::args::MemoryArgument> {
    typedef StringLike ValueCategory;
};
template<>
struct ArgTraits<::s2j::app::args::TimeArgument> {
    typedef StringLike ValueCategory;
};
template<>
struct ArgTraits<::s2j::app::args::AmountArgument> {
    typedef StringLike ValueCategory;
};
}

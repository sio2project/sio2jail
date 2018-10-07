#pragma once

#include <array>
#include <string>

#include "common/Utils.h"

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


/**
 * A simple wrapper that chooses implementation by a name
 */
template<typename InterfaceType>
class ImplementationNameArgument : public TCLAP::Constraint<ImplementationNameArgument<InterfaceType>> {
    // should be const, but sadly TCLAP doens't allow this
    FactoryMap<InterfaceType> factories_;
    std::string description_, implementationName_;

public:
    template<typename DescriptionType, typename DefaultNameType, typename FactoryMapType>
    ImplementationNameArgument(DescriptionType&& description, DefaultNameType& defaultName, FactoryMapType&& factories)
        : factories_(std::forward<FactoryMapType>(factories))
        , description_(std::forward<DescriptionType>(description))
        , implementationName_(std::forward<DefaultNameType>(defaultName)) {}

    ImplementationNameArgument(const ImplementationNameArgument&) = default;
    ImplementationNameArgument(ImplementationNameArgument&&) = default;
    ImplementationNameArgument& operator=(const ImplementationNameArgument&) = default;
    ImplementationNameArgument& operator=(ImplementationNameArgument&&) = default;

    ImplementationNameArgument& operator=(const std::string& name) {
        if (factories_.find(name) == factories_.end()) {
            throw TCLAP::ArgParseException(name + " is not a valid name for " + description_ + ", " + description());
        }
        implementationName_ = name;
        return *this;
    }

    bool check(const ImplementationNameArgument<InterfaceType>& value) const override {
        return factories_.find(value.implementationName_) != factories_.end();
    }

    std::string description() const override {
        std::stringstream ss;
        ss << "value should be one of: ";
        for (auto it = factories_.begin(); it != factories_.end(); ++it) {
            if (it != factories_.begin())
                ss << ", ";
            ss << it->first;
        }
        return ss.str();
    }

    std::string shortID() const override {
        std::stringstream ss;
        for (auto it = factories_.begin(); it != factories_.end(); ++it) {
            if (it != factories_.begin())
                ss << "|";
            ss << it->first;
        }
        return ss.str();
    }

    Factory<InterfaceType> getFactory() const {
        return factories_.at(implementationName_);
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
template<typename InterfaceType>
struct ArgTraits<::s2j::app::args::ImplementationNameArgument<InterfaceType>> {
    typedef StringLike ValueCategory;
};
}

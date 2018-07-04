#pragma once

#include "Exception.h"

#include "logger/Logger.h"

#include <cstring>
#include <algorithm>
#include <functional>
#include <type_traits>

namespace s2j {

template<typename T>
class ResultErrnoPair {
public:
    ResultErrnoPair(T&& t, int errnoCode)
        : value_(std::forward<T>(t)), errnoCode_(errnoCode) {}

    operator T() {
        return value_;
    }

    template<typename U>
    U as() {
        return reinterpret_cast<U>(value_);
    }

    int getErrnoCode() const {
        return errnoCode_;
    }

private:
    T value_;
    int errnoCode_;
};

template<typename Operation, typename ErrnoCollection, typename ...Args>
auto withErrnoCheck(
        const std::string& description,
        const ErrnoCollection& ignoredErrnos,
        Operation&& operation,
        Args&& ...args)
        -> ResultErrnoPair<decltype(operation(args...))> {
    TRACE(description);

    errno = 0;
    auto result = operation(std::forward<Args>(args)...);
    auto returnedErrno = errno;
    logger::trace("Operation returned ", result, " errno ", strerror(returnedErrno));

    using CompareType =
        typename std::conditional<std::numeric_limits<
            decltype(result)>::is_integer,
            decltype(result),
            int64_t
        >::type;

    if (reinterpret_cast<CompareType>(result) < 0) {
        if (std::find(ignoredErrnos.begin(), ignoredErrnos.end(), returnedErrno) == ignoredErrnos.end())
            throw SystemException(description + " failed: " + strerror(returnedErrno), returnedErrno);
    }
    return ResultErrnoPair<decltype(operation(args...))>(std::move(result), returnedErrno);
}

template<typename Operation, typename ...Args>
auto withErrnoCheck(
        const std::string& description,
        std::initializer_list<int> ignoredErrnos,
        Operation&& operation,
        Args&& ...args) {
    return withErrnoCheck<Operation, std::initializer_list<int>, Args...>(
            description,
            ignoredErrnos,
            std::forward<Operation>(operation),
            std::forward<Args>(args)...);
}

template<typename Operation, typename ...Args>
auto withErrnoCheck(const std::string& description, Operation&& operation, Args&& ...args) {
    return withErrnoCheck<Operation, std::initializer_list<int>, Args...>(
            description, {}, std::forward<Operation>(operation), std::forward<Args>(args)...);
}

template<typename Operation, typename ...Args>
auto withErrnoCheck(Operation operation, Args&& ...args)
        -> decltype(operation(args...)) {
    return withErrnoCheck(
            "linux api call", std::forward<Operation>(operation), std::forward<Args>(args)...);
}

}

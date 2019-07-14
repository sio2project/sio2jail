#pragma once

#include "Exception.h"

#include "logger/Logger.h"

#include <algorithm>
#include <cstring>
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

template<
        typename Operation,
        typename ErrnoCollection,
        typename ResultChecker,
        typename... Args>
auto withErrnoCheck(
        const std::string& description,
        const ErrnoCollection& ignoredErrnos,
        ResultChecker&& resultChecker,
        Operation&& operation,
        Args&&... args) -> ResultErrnoPair<decltype(operation(args...))> {
    TRACE(description);

    errno = 0;
    auto result = operation(std::forward<Args>(args)...);
    auto returnedErrno = errno;
    logger::trace(
            "Operation returned ", result, " errno ", strerror(returnedErrno));

    if (!resultChecker(result)) {
        if (std::find(
                    ignoredErrnos.begin(),
                    ignoredErrnos.end(),
                    returnedErrno) == ignoredErrnos.end())
            throw SystemException(
                    description + " failed: " + strerror(returnedErrno),
                    returnedErrno);
    }
    return ResultErrnoPair<decltype(operation(args...))>(
            std::move(result), returnedErrno);
}

template<typename Operation, typename ErrnoCollection, typename... Args>
auto withErrnoCheck(
        const std::string& description,
        const ErrnoCollection& ignoredErrnos,
        Operation&& operation,
        Args&&... args) -> ResultErrnoPair<decltype(operation(args...))> {
    using CompareType = typename std::conditional<
            std::numeric_limits<
                    std::result_of_t<Operation(Args...)>>::is_integer,
            std::result_of_t<Operation(Args...)>,
            int64_t>::type;

    return withErrnoCheck(
            description,
            ignoredErrnos,
            [](auto&& result) -> bool {
                return reinterpret_cast<CompareType>(result) >= 0;
            },
            operation,
            args...);
}

template<typename Operation, typename... Args>
auto withErrnoCheck(
        const std::string& description,
        std::initializer_list<int> ignoredErrnos,
        Operation&& operation,
        Args&&... args) {
    return withErrnoCheck<Operation, std::initializer_list<int>, Args...>(
            description,
            ignoredErrnos,
            std::forward<Operation>(operation),
            std::forward<Args>(args)...);
}

template<typename Operation, typename ResultChecker, typename... Args>
auto withGuardedErrnoCheck(
        const std::string& description,
        ResultChecker&& resultChecker,
        Operation&& operation,
        Args&&... args) {
    return withErrnoCheck<
            Operation,
            std::initializer_list<int>,
            ResultChecker,
            Args...>(
            description,
            {},
            std::forward<ResultChecker>(resultChecker),
            std::forward<Operation>(operation),
            std::forward<Args>(args)...);
}

template<typename Operation, typename... Args>
auto withErrnoCheck(
        const std::string& description,
        Operation&& operation,
        Args&&... args) {
    return withErrnoCheck<Operation, std::initializer_list<int>, Args...>(
            description,
            {},
            std::forward<Operation>(operation),
            std::forward<Args>(args)...);
}

template<typename Operation, typename... Args>
auto withErrnoCheck(Operation operation, Args&&... args)
        -> decltype(operation(args...)) {
    return withErrnoCheck(
            "linux api call",
            std::forward<Operation>(operation),
            std::forward<Args>(args)...);
}

} // namespace s2j
